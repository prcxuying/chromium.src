// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_NATIVE_ANDROID_PROTOCOL_HANDLER_H_
#define ANDROID_WEBVIEW_NATIVE_ANDROID_PROTOCOL_HANDLER_H_

#include "base/android/jni_android.h"
#include "base/memory/scoped_ptr.h"

namespace net {
class URLRequestContext;
class URLRequestInterceptor;
}  // namespace net

namespace android_webview {

// These methods create interceptors for Android WebView-specific schemes:
//
//  - "content:" scheme is used for accessing data from Android content
//    providers, see http://developer.android.com/guide/topics/providers/
//      content-provider-basics.html#ContentURIs
scoped_ptr<net::URLRequestInterceptor> CreateContentSchemeRequestInterceptor();

//  - "file:" scheme extension for accessing application assets and resources
//    (file:///android_asset/ and file:///android_res/), see
//    http://developer.android.com/reference/android/webkit/
//      WebSettings.html#setAllowFileAccess(boolean)
scoped_ptr<net::URLRequestInterceptor> CreateAssetFileRequestInterceptor();

bool RegisterAndroidProtocolHandler(JNIEnv* env);

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_NATIVE_ANDROID_PROTOCOL_HANDLER_H_
