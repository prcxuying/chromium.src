# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'gfx_geometry',
      'type': '<(component)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'defines': [
        'GFX_IMPLEMENTATION',
      ],
      'sources': [
        'geometry/box_f.cc',
        'geometry/box_f.h',
        'geometry/cubic_bezier.h',
        'geometry/cubic_bezier.cc',
        'geometry/insets.cc',
        'geometry/insets.h',
        'geometry/insets_base.h',
        'geometry/insets_f.cc',
        'geometry/insets_f.h',
        'geometry/matrix3_f.cc',
        'geometry/matrix3_f.h',
        'geometry/point.cc',
        'geometry/point.h',
        'geometry/point3_f.cc',
        'geometry/point3_f.h',
        'geometry/point_conversions.cc',
        'geometry/point_conversions.h',
        'geometry/point_f.cc',
        'geometry/point_f.h',
        'geometry/quad_f.cc',
        'geometry/quad_f.h',
        'geometry/rect.cc',
        'geometry/rect.h',
        'geometry/rect_conversions.cc',
        'geometry/rect_conversions.h',
        'geometry/rect_f.cc',
        'geometry/rect_f.h',
        'geometry/r_tree.h',
        'geometry/r_tree_base.cc',
        'geometry/r_tree_base.h',
        'geometry/safe_integer_conversions.h',
        'geometry/scroll_offset.cc',
        'geometry/scroll_offset.h',
        'geometry/size.cc',
        'geometry/size.h',
        'geometry/size_conversions.cc',
        'geometry/size_conversions.h',
        'geometry/size_f.cc',
        'geometry/size_f.h',
        'geometry/vector2d.cc',
        'geometry/vector2d.h',
        'geometry/vector2d_conversions.cc',
        'geometry/vector2d_conversions.h',
        'geometry/vector2d_f.cc',
        'geometry/vector2d_f.h',
        'geometry/vector3d_f.cc',
        'geometry/vector3d_f.h',
      ],
      # TODO(jdduke): Revisit optimization after gauging benefit, crbug/419051.
      'includes': [
        '../../build/android/increase_size_for_speed.gypi',
      ],
    },
    {
      'target_name': 'gfx',
      'type': '<(component)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/base/base.gyp:base_static',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/harfbuzz-ng/harfbuzz.gyp:harfbuzz-ng',
        '<(DEPTH)/third_party/icu/icu.gyp:icui18n',
        '<(DEPTH)/third_party/icu/icu.gyp:icuuc',
        '<(DEPTH)/third_party/libpng/libpng.gyp:libpng',
        '<(DEPTH)/third_party/zlib/zlib.gyp:zlib',
        'gfx_geometry',
      ],
      # text_elider.h includes ICU headers.
      'export_dependent_settings': [
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/icu/icu.gyp:icui18n',
        '<(DEPTH)/third_party/icu/icu.gyp:icuuc',
      ],
      'defines': [
        'GFX_IMPLEMENTATION',
      ],
      'include_dirs': [
        '<(DEPTH)/third_party/icu/source/common'
      ],
      'sources': [
        'android/device_display_info.cc',
        'android/device_display_info.h',
        'android/gfx_jni_registrar.cc',
        'android/gfx_jni_registrar.h',
        'android/java_bitmap.cc',
        'android/java_bitmap.h',
        'android/shared_device_display_info.cc',
        'android/shared_device_display_info.h',
        'android/view_configuration.cc',
        'android/view_configuration.h',
        'animation/animation.cc',
        'animation/animation.h',
        'animation/animation_container.cc',
        'animation/animation_container.h',
        'animation/animation_container_element.h',
        'animation/animation_container_observer.h',
        'animation/animation_delegate.h',
        'animation/linear_animation.cc',
        'animation/linear_animation.h',
        'animation/multi_animation.cc',
        'animation/multi_animation.h',
        'animation/slide_animation.cc',
        'animation/slide_animation.h',
        'animation/throb_animation.cc',
        'animation/throb_animation.h',
        'animation/tween.cc',
        'animation/tween.h',
        'blit.cc',
        'blit.h',
        'break_list.h',
        'canvas.cc',
        'canvas.h',
        'canvas_notimplemented.cc',
        'canvas_paint_mac.h',
        'canvas_paint_mac.mm',
        'canvas_paint_win.cc',
        'canvas_paint_win.h',
        'canvas_skia.cc',
        'canvas_skia_paint.h',
        'codec/jpeg_codec.cc',
        'codec/jpeg_codec.h',
        'codec/png_codec.cc',
        'codec/png_codec.h',
        'color_analysis.cc',
        'color_analysis.h',
        'color_profile.cc',
        'color_profile.h',
        'color_profile_mac.mm',
        'color_profile_win.cc',
        'color_utils.cc',
        'color_utils.h',
        'display.cc',
        'display.h',
        'display_change_notifier.cc',
        'display_change_notifier.h',
        'display_observer.cc',
        'display_observer.h',
        'favicon_size.cc',
        'favicon_size.h',
        'font.cc',
        'font.h',
        'font_fallback.h',
        'font_fallback_linux.cc',
        'font_fallback_mac.cc',
        'font_fallback_win.cc',
        'font_fallback_win.h',
        'font_list.cc',
        'font_list.h',
        'font_list_impl.cc',
        'font_list_impl.h',
        'font_render_params.cc',
        'font_render_params.h',
        'font_render_params_android.cc',
        'font_render_params_linux.cc',
        'font_render_params_mac.cc',
        'font_render_params_win.cc',
        'frame_time.h',
        'gfx_export.h',
        'gfx_paths.cc',
        'gfx_paths.h',
        'gpu_memory_buffer.cc',
        'gpu_memory_buffer.h',
        'image/canvas_image_source.cc',
        'image/canvas_image_source.h',
        'image/image.cc',
        'image/image.h',
        'image/image_family.cc',
        'image/image_family.h',
        'image/image_ios.mm',
        'image/image_mac.mm',
        'image/image_png_rep.cc',
        'image/image_png_rep.h',
        'image/image_skia.cc',
        'image/image_skia.h',
        'image/image_skia_operations.cc',
        'image/image_skia_operations.h',
        'image/image_skia_rep.cc',
        'image/image_skia_rep.h',
        'image/image_skia_source.h',
        'image/image_skia_util_ios.h',
        'image/image_skia_util_ios.mm',
        'image/image_skia_util_mac.h',
        'image/image_skia_util_mac.mm',
        'image/image_util.cc',
        'image/image_util.h',
        'image/image_util_ios.mm',
        'interpolated_transform.cc',
        'interpolated_transform.h',
        'linux_font_delegate.cc',
        'linux_font_delegate.h',
        'mac/coordinate_conversion.h',
        'mac/coordinate_conversion.mm',
        'mac/scoped_ns_disable_screen_updates.h',
        'native_widget_types.h',
        'nine_image_painter.cc',
        'nine_image_painter.h',
        'overlay_transform.h',
        'pango_util.cc',
        'pango_util.h',
        'path.cc',
        'path.h',
        'path_aura.cc',
        'path_win.cc',
        'path_win.h',
        'path_x11.cc',
        'path_x11.h',
        'platform_font.h',
        'platform_font_android.cc',
        'platform_font_ios.h',
        'platform_font_ios.mm',
        'platform_font_mac.h',
        'platform_font_mac.mm',
        'platform_font_ozone.cc',
        'platform_font_pango.cc',
        'platform_font_pango.h',
        'platform_font_win.cc',
        'platform_font_win.h',
        'range/range.cc',
        'range/range.h',
        'range/range_mac.mm',
        'range/range_win.cc',
        'render_text.cc',
        'render_text.h',
        'render_text_harfbuzz.cc',
        'render_text_harfbuzz.h',
        'render_text_mac.cc',
        'render_text_mac.h',
        'render_text_ozone.cc',
        'render_text_pango.cc',
        'render_text_pango.h',
        'render_text_win.cc',
        'render_text_win.h',
        'scoped_canvas.h',
        'scoped_cg_context_save_gstate_mac.h',
        'scoped_ns_graphics_context_save_gstate_mac.h',
        'scoped_ns_graphics_context_save_gstate_mac.mm',
        'scoped_ui_graphics_push_context_ios.h',
        'scoped_ui_graphics_push_context_ios.mm',
        'screen.cc',
        'screen.h',
        'screen_android.cc',
        'screen_aura.cc',
        'screen_ios.mm',
        'screen_mac.mm',
        'screen_win.cc',
        'screen_win.h',
        'scrollbar_size.cc',
        'scrollbar_size.h',
        'selection_model.cc',
        'selection_model.h',
        'sequential_id_generator.cc',
        'sequential_id_generator.h',
        'shadow_value.cc',
        'shadow_value.h',
        'skbitmap_operations.cc',
        'skbitmap_operations.h',
        'skia_util.cc',
        'skia_util.h',
        'switches.cc',
        'switches.h',
        'sys_color_change_listener.cc',
        'sys_color_change_listener.h',
        'text_constants.h',
        'text_elider.cc',
        'text_elider.h',
        'text_utils.cc',
        'text_utils.h',
        'text_utils_android.cc',
        'text_utils_ios.mm',
        'text_utils_skia.cc',
        'transform.cc',
        'transform.h',
        'transform_util.cc',
        'transform_util.h',
        'ui_gfx_exports.cc',
        'utf16_indexing.cc',
        'utf16_indexing.h',
        'vsync_provider.h',
        'win/direct_write.cc',
        'win/direct_write.h',
        'win/dpi.cc',
        'win/dpi.h',
        'win/hwnd_util.cc',
        'win/hwnd_util.h',
        'win/scoped_set_map_mode.h',
        'win/singleton_hwnd.cc',
        'win/singleton_hwnd.h',
        'win/window_impl.cc',
        'win/window_impl.h',
      ],
      'includes': [
        '../../build/android/increase_size_for_speed.gypi',
      ],
      'conditions': [
        ['OS=="ios"', {
          'dependencies': [
            '<(DEPTH)/ui/ios/ui_ios.gyp:ui_ios',
          ],
          # iOS only uses a subset of UI.
          'sources/': [
            ['exclude', '^codec/jpeg_codec\\.cc$'],
          ],
        }, {
          'dependencies': [
            '<(libjpeg_gyp_path):libjpeg',
          ],
        }],
        # TODO(asvitkine): Switch all platforms to use canvas_skia.cc.
        #                  http://crbug.com/105550
        ['use_canvas_skia==1', {
          'sources!': [
            'canvas_notimplemented.cc',
          ],
        }, {  # use_canvas_skia!=1
          'sources!': [
            'canvas_skia.cc',
          ],
        }],
        ['OS=="win"', {
          'sources': [
            'gdi_util.cc',
            'gdi_util.h',
            'icon_util.cc',
            'icon_util.h',
          ],
          # TODO(jschuh): C4267: http://crbug.com/167187 size_t -> int
          # C4324 is structure was padded due to __declspec(align()), which is
          # uninteresting.
          'msvs_disabled_warnings': [ 4267, 4324 ],
        }],
        ['OS=="android"', {
          'sources!': [
            'animation/throb_animation.cc',
            'display_observer.cc',
            'selection_model.cc',
          ],
          'dependencies': [
            'gfx_jni_headers',
          ],
          'link_settings': {
            'libraries': [
              '-landroid',
              '-ljnigraphics',
            ],
          },
        }],
        ['use_aura==0 and toolkit_views==0', {
          'sources!': [
            'nine_image_painter.cc',
            'nine_image_painter.h',
          ],
        }],
        ['OS=="android" and use_aura==0', {
          'sources!': [
            'path.cc',
          ],
        }],
        ['OS=="android" and use_aura==1', {
          'sources!': [
            'screen_android.cc',
          ],
        }],
        ['OS=="android" and android_webview_build==0', {
          'dependencies': [
            '<(DEPTH)/base/base.gyp:base_java',
          ],
        }],
        ['OS=="android" or OS=="ios"', {
          'sources!': [
            'render_text.cc',
            'render_text.h',
            'render_text_harfbuzz.cc',
            'render_text_harfbuzz.h',
            'text_utils_skia.cc',
          ],
        }],
        ['use_x11==1', {
          'dependencies': [
            '../../build/linux/system.gyp:x11',
            'x/gfx_x11.gyp:gfx_x11',
          ],
        }],
        ['use_pango==1', {
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:pangocairo',
          ],
          'sources!': [
            'platform_font_ozone.cc',
            'render_text_ozone.cc',
          ],
        }],
        ['desktop_linux==1 or chromeos==1', {
          'dependencies': [
            # font_render_params_linux.cc uses fontconfig
            '<(DEPTH)/build/linux/system.gyp:fontconfig',
          ],
        }],
      ],
      'target_conditions': [
        # Need 'target_conditions' to override default filename_rules to include
        # the file on iOS.
        ['OS == "ios"', {
          'sources/': [
            ['include', '^scoped_cg_context_save_gstate_mac\\.h$'],
          ],
        }],
      ],
    },
    {
      'target_name': 'gfx_test_support',
      'type': 'static_library',
      'sources': [
        'image/image_unittest_util.cc',
        'image/image_unittest_util.h',
        'image/image_unittest_util_ios.mm',
        'image/image_unittest_util_mac.mm',
        'test/fontconfig_util_linux.cc',
        'test/fontconfig_util_linux.h',
        'test/gfx_util.cc',
        'test/gfx_util.h',
        'test/ui_cocoa_test_helper.h',
        'test/ui_cocoa_test_helper.mm',
      ],
      'dependencies': [
        '../../base/base.gyp:base',
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
      ],
      'conditions': [
        ['OS == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
            ],
          },
        }],
        ['OS=="ios"', {
          # The cocoa files don't apply to iOS.
          'sources/': [
            ['exclude', 'cocoa']
          ],
        }],
        ['OS=="linux"', {
          'dependencies': [
            '../../build/linux/system.gyp:fontconfig',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['OS=="android"' , {
     'targets': [
       {
         'target_name': 'gfx_jni_headers',
         'type': 'none',
         'sources': [
           '../android/java/src/org/chromium/ui/gfx/BitmapHelper.java',
           '../android/java/src/org/chromium/ui/gfx/DeviceDisplayInfo.java',
           '../android/java/src/org/chromium/ui/gfx/ViewConfigurationHelper.java',
         ],
         'variables': {
           'jni_gen_package': 'ui/gfx',
         },
         'includes': [ '../../build/jni_generator.gypi' ],
       },
     ],
    }],
  ],
}
