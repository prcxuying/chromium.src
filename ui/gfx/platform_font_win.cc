// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/platform_font_win.h"

#include <algorithm>
#include <dwrite.h>
#include <math.h>
#include <windows.h>

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_comptr.h"
#include "base/win/scoped_gdi_object.h"
#include "base/win/scoped_hdc.h"
#include "base/win/scoped_select_object.h"
#include "base/win/win_util.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font.h"
#include "ui/gfx/font_render_params.h"
#include "ui/gfx/win/scoped_set_map_mode.h"

namespace {

// If the tmWeight field of a TEXTMETRIC structure has a value >= this, the
// font is bold.
const int kTextMetricWeightBold = 700;

// Returns the minimum font size, using the minimum size callback, if set.
int GetMinimumFontSize() {
  int min_font_size = 0;
  if (gfx::PlatformFontWin::get_minimum_font_size_callback)
    min_font_size = gfx::PlatformFontWin::get_minimum_font_size_callback();
  return min_font_size;
}

// Returns either minimum font allowed for a current locale or
// lf_height + size_delta value.
int AdjustFontSize(int lf_height, int size_delta) {
  if (lf_height < 0) {
    lf_height -= size_delta;
  } else {
    lf_height += size_delta;
  }
  const int min_font_size = GetMinimumFontSize();
  // Make sure lf_height is not smaller than allowed min font size for current
  // locale.
  if (abs(lf_height) < min_font_size) {
    return lf_height < 0 ? -min_font_size : min_font_size;
  } else {
    return lf_height;
  }
}

// Sets style properties on |font_info| based on |font_style|.
void SetLogFontStyle(int font_style, LOGFONT* font_info) {
  font_info->lfUnderline = (font_style & gfx::Font::UNDERLINE) != 0;
  font_info->lfItalic = (font_style & gfx::Font::ITALIC) != 0;
  font_info->lfWeight = (font_style & gfx::Font::BOLD) ? FW_BOLD : FW_NORMAL;
}

void GetTextMetricsForFont(HDC hdc, HFONT font, TEXTMETRIC* text_metrics) {
  base::win::ScopedSelectObject scoped_font(hdc, font);
  GetTextMetrics(hdc, text_metrics);
}

// Returns a matching IDWriteFont for the |face_name| passed in. If we fail
// to find a matching font, then we return the IDWriteFont corresponding to
// the default font on the system.
// Returns S_OK on success.
HRESULT GetMatchingDirectWriteFontForTypeface(const wchar_t* face_name,
                                              int font_style,
                                              IDWriteFactory* factory,
                                              IDWriteFont** dwrite_font) {
  // Enumerate the system font collectione exposed by DirectWrite for a
  // matching font.
  base::win::ScopedComPtr<IDWriteFontCollection> font_collection;
  HRESULT hr = factory->GetSystemFontCollection(font_collection.Receive());
  if (FAILED(hr)) {
    CHECK(false);
    return hr;
  }

  base::win::ScopedComPtr<IDWriteFontFamily> font_family;
  BOOL exists = FALSE;
  uint32 index = 0;
  hr = font_collection->FindFamilyName(face_name, &index, &exists);
  // If we fail to find a match then fallback to the default font on the
  // system. This is what skia does as well.
  if (FAILED(hr)) {
    NONCLIENTMETRICS metrics = {0};
    metrics.cbSize = sizeof(metrics);
    if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,
                               sizeof(metrics),
                               &metrics,
                               0)) {
      CHECK(false);
      return E_FAIL;
    }
    hr = font_collection->FindFamilyName(metrics.lfMessageFont.lfFaceName,
                                         &index,
                                         &exists);
  }

  if (FAILED(hr)) {
    CHECK(false);
    return hr;
  }

  hr = font_collection->GetFontFamily(index, font_family.Receive());
  if (FAILED(hr)) {
    CHECK(false);
    return hr;
  }

  DWRITE_FONT_WEIGHT weight = (font_style & SkTypeface::kBold)
                            ? DWRITE_FONT_WEIGHT_BOLD
                            : DWRITE_FONT_WEIGHT_NORMAL;
  DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
  DWRITE_FONT_STYLE italic = (font_style & SkTypeface::kItalic)
                            ? DWRITE_FONT_STYLE_ITALIC
                            : DWRITE_FONT_STYLE_NORMAL;
  hr = font_family->GetFirstMatchingFont(weight, stretch, italic,
                                         dwrite_font);
  if (FAILED(hr))
    CHECK(false);
  return hr;
}

}  // namespace

namespace gfx {

// static
PlatformFontWin::HFontRef* PlatformFontWin::base_font_ref_;

// static
PlatformFontWin::AdjustFontCallback
    PlatformFontWin::adjust_font_callback = nullptr;
PlatformFontWin::GetMinimumFontSizeCallback
    PlatformFontWin::get_minimum_font_size_callback = NULL;

IDWriteFactory* PlatformFontWin::direct_write_factory_ = nullptr;

////////////////////////////////////////////////////////////////////////////////
// PlatformFontWin, public

PlatformFontWin::PlatformFontWin() : font_ref_(GetBaseFontRef()) {
}

PlatformFontWin::PlatformFontWin(NativeFont native_font) {
  InitWithCopyOfHFONT(native_font);
}

PlatformFontWin::PlatformFontWin(const std::string& font_name,
                                 int font_size) {
  InitWithFontNameAndSize(font_name, font_size);
}

Font PlatformFontWin::DeriveFontWithHeight(int height, int style) {
  DCHECK_GE(height, 0);
  if (GetHeight() == height && GetStyle() == style)
    return Font(this);

  // CreateFontIndirect() doesn't return the largest size for the given height
  // when decreasing the height. Iterate to find it.
  if (GetHeight() > height) {
    const int min_font_size = GetMinimumFontSize();
    Font font = DeriveFont(-1, style);
    int font_height = font.GetHeight();
    int font_size = font.GetFontSize();
    while (font_height > height && font_size != min_font_size) {
      font = font.Derive(-1, style);
      if (font_height == font.GetHeight() && font_size == font.GetFontSize())
        break;
      font_height = font.GetHeight();
      font_size = font.GetFontSize();
    }
    return font;
  }

  LOGFONT font_info;
  GetObject(GetNativeFont(), sizeof(LOGFONT), &font_info);
  font_info.lfHeight = height;
  SetLogFontStyle(style, &font_info);

  HFONT hfont = CreateFontIndirect(&font_info);
  return DeriveWithCorrectedSize(hfont);
}

////////////////////////////////////////////////////////////////////////////////
// PlatformFontWin, PlatformFont implementation:

Font PlatformFontWin::DeriveFont(int size_delta, int style) const {
  LOGFONT font_info;
  GetObject(GetNativeFont(), sizeof(LOGFONT), &font_info);
  const int requested_font_size = font_ref_->requested_font_size();
  font_info.lfHeight = AdjustFontSize(-requested_font_size, size_delta);
  SetLogFontStyle(style, &font_info);

  HFONT hfont = CreateFontIndirect(&font_info);
  return Font(new PlatformFontWin(CreateHFontRef(hfont)));
}

int PlatformFontWin::GetHeight() const {
  return font_ref_->height();
}

int PlatformFontWin::GetBaseline() const {
  return font_ref_->baseline();
}

int PlatformFontWin::GetCapHeight() const {
  return font_ref_->cap_height();
}

int PlatformFontWin::GetExpectedTextWidth(int length) const {
  return length * std::min(font_ref_->GetDluBaseX(),
                           font_ref_->ave_char_width());
}

int PlatformFontWin::GetStyle() const {
  return font_ref_->style();
}

std::string PlatformFontWin::GetFontName() const {
  return font_ref_->font_name();
}

std::string PlatformFontWin::GetActualFontNameForTesting() const {
  // With the current implementation on Windows, HFontRef::font_name() returns
  // the font name taken from the HFONT handle, but it's not the name that comes
  // from the font's metadata.  See http://crbug.com/327287
  return font_ref_->font_name();
}

std::string PlatformFontWin::GetLocalizedFontName() const {
  base::win::ScopedCreateDC memory_dc(CreateCompatibleDC(NULL));
  if (!memory_dc.Get())
    return GetFontName();

  // When a font has a localized name for a language matching the system
  // locale, GetTextFace() returns the localized name.
  base::win::ScopedSelectObject font(memory_dc.Get(), font_ref_->hfont());
  wchar_t localized_font_name[LF_FACESIZE];
  int length = GetTextFace(memory_dc.Get(), arraysize(localized_font_name),
                           &localized_font_name[0]);
  if (length <= 0)
    return GetFontName();
  return base::SysWideToUTF8(localized_font_name);
}

int PlatformFontWin::GetFontSize() const {
  return font_ref_->font_size();
}

const FontRenderParams& PlatformFontWin::GetFontRenderParams() const {
  CR_DEFINE_STATIC_LOCAL(const gfx::FontRenderParams, params,
      (gfx::GetFontRenderParams(gfx::FontRenderParamsQuery(false), NULL)));
  return params;
}

NativeFont PlatformFontWin::GetNativeFont() const {
  return font_ref_->hfont();
}

void PlatformFontWin::SetDirectWriteFactory(IDWriteFactory* factory) {
  // We grab a reference on the DirectWrite factory. This reference is
  // leaked, which is ok because skia leaks it as well.
  factory->AddRef();
  direct_write_factory_ = factory;
}

////////////////////////////////////////////////////////////////////////////////
// Font, private:

void PlatformFontWin::InitWithCopyOfHFONT(HFONT hfont) {
  DCHECK(hfont);
  LOGFONT font_info;
  GetObject(hfont, sizeof(LOGFONT), &font_info);
  font_ref_ = CreateHFontRef(CreateFontIndirect(&font_info));
}

void PlatformFontWin::InitWithFontNameAndSize(const std::string& font_name,
                                              int font_size) {
  HFONT hf = ::CreateFont(-font_size, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY,
                          DEFAULT_PITCH | FF_DONTCARE,
                          base::UTF8ToUTF16(font_name).c_str());
  font_ref_ = CreateHFontRef(hf);
}

// static
PlatformFontWin::HFontRef* PlatformFontWin::GetBaseFontRef() {
  if (base_font_ref_ == NULL) {
    NONCLIENTMETRICS_XP metrics;
    base::win::GetNonClientMetrics(&metrics);

    if (adjust_font_callback)
      adjust_font_callback(&metrics.lfMessageFont);
    metrics.lfMessageFont.lfHeight =
        AdjustFontSize(metrics.lfMessageFont.lfHeight, 0);
    HFONT font = CreateFontIndirect(&metrics.lfMessageFont);
    DLOG_ASSERT(font);
    base_font_ref_ = PlatformFontWin::CreateHFontRef(font);
    // base_font_ref_ is global, up the ref count so it's never deleted.
    base_font_ref_->AddRef();
  }
  return base_font_ref_;
}

PlatformFontWin::HFontRef* PlatformFontWin::CreateHFontRef(HFONT font) {
  if (direct_write_factory_)
    return CreateHFontRefFromSkia(font);

  TEXTMETRIC font_metrics;

  {
    base::win::ScopedGetDC screen_dc(NULL);
    gfx::ScopedSetMapMode mode(screen_dc, MM_TEXT);
    GetTextMetricsForFont(screen_dc, font, &font_metrics);
  }

  return CreateHFontRef(font, font_metrics);
}

PlatformFontWin::HFontRef* PlatformFontWin::CreateHFontRef(
    HFONT font,
    const TEXTMETRIC& font_metrics) {
  const int height = std::max<int>(1, font_metrics.tmHeight);
  const int baseline = std::max<int>(1, font_metrics.tmAscent);
  const int cap_height =
      std::max<int>(1, font_metrics.tmAscent - font_metrics.tmInternalLeading);
  const int ave_char_width = std::max<int>(1, font_metrics.tmAveCharWidth);
  const int font_size =
      std::max<int>(1, font_metrics.tmHeight - font_metrics.tmInternalLeading);
  int style = 0;
  if (font_metrics.tmItalic)
    style |= Font::ITALIC;
  if (font_metrics.tmUnderlined)
    style |= Font::UNDERLINE;
  if (font_metrics.tmWeight >= kTextMetricWeightBold)
    style |= Font::BOLD;

  return new HFontRef(font, font_size, height, baseline, cap_height,
                      ave_char_width, style);
}

Font PlatformFontWin::DeriveWithCorrectedSize(HFONT base_font) {
  base::win::ScopedGetDC screen_dc(NULL);
  gfx::ScopedSetMapMode mode(screen_dc, MM_TEXT);

  base::win::ScopedGDIObject<HFONT> best_font(base_font);
  TEXTMETRIC best_font_metrics;
  GetTextMetricsForFont(screen_dc, best_font, &best_font_metrics);

  LOGFONT font_info;
  GetObject(base_font, sizeof(LOGFONT), &font_info);

  // Set |lfHeight| to negative value to indicate it's the size, not the height.
  font_info.lfHeight =
      -(best_font_metrics.tmHeight - best_font_metrics.tmInternalLeading);

  do {
    // Increment font size. Prefer font with greater size if its height isn't
    // greater than height of base font.
    font_info.lfHeight = AdjustFontSize(font_info.lfHeight, 1);
    base::win::ScopedGDIObject<HFONT> font(CreateFontIndirect(&font_info));
    TEXTMETRIC font_metrics;
    GetTextMetricsForFont(screen_dc, font, &font_metrics);
    if (font_metrics.tmHeight > best_font_metrics.tmHeight)
      break;
    best_font.Set(font.release());
    best_font_metrics = font_metrics;
  } while (true);

  return Font(new PlatformFontWin(CreateHFontRef(best_font.release())));
}

// static
PlatformFontWin::HFontRef* PlatformFontWin::CreateHFontRefFromSkia(
    HFONT gdi_font) {
  LOGFONT font_info = {0};
  GetObject(gdi_font, sizeof(LOGFONT), &font_info);

  int skia_style = SkTypeface::kNormal;
  if (font_info.lfWeight >= FW_SEMIBOLD &&
      font_info.lfWeight <= FW_ULTRABOLD) {
    skia_style |= SkTypeface::kBold;
  }
  if (font_info.lfItalic)
    skia_style |= SkTypeface::kItalic;

  // Skia does not return all values we need for font metrics. For e.g.
  // the cap height which indicates the height of capital letters is not
  // returned even though it is returned by DirectWrite.
  // TODO(ananta)
  // Fix SkScalerContext_win_dw.cpp to return all metrics we need from
  // DirectWrite and remove the code here which retrieves metrics from
  // DirectWrite to calculate the cap height.
  base::win::ScopedComPtr<IDWriteFont> dwrite_font;
  HRESULT hr = GetMatchingDirectWriteFontForTypeface(font_info.lfFaceName,
                                                     skia_style,
                                                     direct_write_factory_,
                                                     dwrite_font.Receive());
  if (FAILED(hr)) {
    CHECK(false);
    return nullptr;
  }

  DWRITE_FONT_METRICS dwrite_font_metrics = {0};
  dwrite_font->GetMetrics(&dwrite_font_metrics);

  skia::RefPtr<SkTypeface> skia_face = skia::AdoptRef(
      SkTypeface::CreateFromName(
          base::SysWideToUTF8(font_info.lfFaceName).c_str(),
                              static_cast<SkTypeface::Style>(skia_style)));

  BOOL antialiasing = TRUE;
  SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &antialiasing, 0);

  SkPaint paint;
  paint.setAntiAlias(!!antialiasing);
  paint.setTypeface(skia_face.get());
  paint.setTextSize(abs(font_info.lfHeight));
  SkPaint::FontMetrics skia_metrics;
  paint.getFontMetrics(&skia_metrics);

  // The calculations below are similar to those in the CreateHFontRef
  // function.
  const int height = std::round(skia_metrics.fDescent - skia_metrics.fAscent);
  const int baseline = std::max<int>(1, std::round(-skia_metrics.fAscent));
  const int cap_height = std::round(-font_info.lfHeight *
      dwrite_font_metrics.capHeight / dwrite_font_metrics.designUnitsPerEm);

  // The metrics retrieved from skia don't have the average character width. In
  // any case if we get the average character width from skia then use that or
  // use the text extent technique as documented by microsoft. See
  // GetAverageCharWidthInDialogUnits for details.
  const int ave_char_width =
      skia_metrics.fAvgCharWidth == 0 ?
          HFontRef::GetAverageCharWidthInDialogUnits(gdi_font)
              : skia_metrics.fAvgCharWidth;

  // tmAscent - tmInternalLeading in gdi font land gives us the cap height.
  // We can use fAscent - cap_height in DirectWrite land to get the internal
  // leading value.
  const int internal_leading = -skia_metrics.fAscent - cap_height;
  const int font_size = std::max<int>(1, height - internal_leading);

  int style = 0;
  if (skia_style & SkTypeface::kItalic)
    style |= Font::ITALIC;
  if (font_info.lfUnderline)
    style |= Font::UNDERLINE;
  if (font_info.lfWeight >= kTextMetricWeightBold)
    style |= Font::BOLD;
  return new HFontRef(gdi_font, font_size, height, baseline, cap_height,
                      ave_char_width, style);
}

PlatformFontWin::PlatformFontWin(HFontRef* hfont_ref) : font_ref_(hfont_ref) {
}

////////////////////////////////////////////////////////////////////////////////
// PlatformFontWin::HFontRef:

PlatformFontWin::HFontRef::HFontRef(HFONT hfont,
                                    int font_size,
                                    int height,
                                    int baseline,
                                    int cap_height,
                                    int ave_char_width,
                                    int style)
    : hfont_(hfont),
      font_size_(font_size),
      height_(height),
      baseline_(baseline),
      cap_height_(cap_height),
      ave_char_width_(ave_char_width),
      style_(style),
      dlu_base_x_(-1),
      requested_font_size_(font_size) {
  DLOG_ASSERT(hfont);

  LOGFONT font_info;
  GetObject(hfont_, sizeof(LOGFONT), &font_info);
  font_name_ = base::UTF16ToUTF8(base::string16(font_info.lfFaceName));
  if (font_info.lfHeight < 0)
    requested_font_size_ = -font_info.lfHeight;
}

int PlatformFontWin::HFontRef::GetDluBaseX() {
  if (dlu_base_x_ != -1)
    return dlu_base_x_;

  dlu_base_x_ = GetAverageCharWidthInDialogUnits(hfont_);
  return dlu_base_x_;
}

// static
int PlatformFontWin::HFontRef::GetAverageCharWidthInDialogUnits(
    HFONT gdi_font) {
  base::win::ScopedGetDC screen_dc(NULL);
  base::win::ScopedSelectObject font(screen_dc, gdi_font);
  gfx::ScopedSetMapMode mode(screen_dc, MM_TEXT);

  // Yes, this is how Microsoft recommends calculating the dialog unit
  // conversions. See: http://support.microsoft.com/kb/125681
  SIZE ave_text_size;
  GetTextExtentPoint32(screen_dc,
                       L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
                       52, &ave_text_size);
  int dlu_base_x = (ave_text_size.cx / 26 + 1) / 2;

  DCHECK_NE(dlu_base_x, -1);
  return dlu_base_x;
}

PlatformFontWin::HFontRef::~HFontRef() {
  DeleteObject(hfont_);
}

////////////////////////////////////////////////////////////////////////////////
// PlatformFont, public:

// static
PlatformFont* PlatformFont::CreateDefault() {
  return new PlatformFontWin;
}

// static
PlatformFont* PlatformFont::CreateFromNativeFont(NativeFont native_font) {
  return new PlatformFontWin(native_font);
}

// static
PlatformFont* PlatformFont::CreateFromNameAndSize(const std::string& font_name,
                                                  int font_size) {
  return new PlatformFontWin(font_name, font_size);
}

}  // namespace gfx
