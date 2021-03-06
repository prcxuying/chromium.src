// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_ANDROID_TAB_MODEL_TAB_MODEL_JNI_BRIDGE_H_
#define CHROME_BROWSER_UI_ANDROID_TAB_MODEL_TAB_MODEL_JNI_BRIDGE_H_

#include <jni.h>
#include <vector>

#include "base/android/jni_weak_ref.h"
#include "base/android/scoped_java_ref.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"

class Profile;
class TabAndroid;

namespace content {
class WebContents;
}

// Bridges calls between the C++ and the Java TabModels. Functions in this
// class should do little more than make calls out to the Java TabModel, which
// is what actually stores Tabs.
class TabModelJniBridge : public TabModel {
 public:
  TabModelJniBridge(JNIEnv* env, jobject obj, bool is_incognito);
  void Destroy(JNIEnv* env, jobject obj);
  virtual ~TabModelJniBridge();

  // Registers the JNI bindings.
  static bool Register(JNIEnv* env);

  // Called by JNI
  base::android::ScopedJavaLocalRef<jobject> GetProfileAndroid(JNIEnv* env,
                                                               jobject obj);
  void TabAddedToModel(JNIEnv* env, jobject obj, jobject jtab);

  // TabModel::
  virtual int GetTabCount() const override;
  virtual int GetActiveIndex() const override;
  virtual content::WebContents* GetWebContentsAt(int index) const override;
  virtual TabAndroid* GetTabAt(int index) const override;

  virtual void SetActiveIndex(int index) override;
  virtual void CloseTabAt(int index) override;

  virtual void CreateTab(content::WebContents* web_contents,
                         int parent_tab_id) override;

  virtual content::WebContents* CreateNewTabForDevTools(
      const GURL& url) override;

  // Return true if we are currently restoring sessions asynchronously.
  virtual bool IsSessionRestoreInProgress() const override;

  // Instructs the TabModel to broadcast a notification that all tabs are now
  // loaded from storage.
  void BroadcastSessionRestoreComplete(JNIEnv* env, jobject obj);

 protected:
  JavaObjectWeakGlobalRef java_object_;

 private:
  DISALLOW_COPY_AND_ASSIGN(TabModelJniBridge);
};

#endif  // CHROME_BROWSER_UI_ANDROID_TAB_MODEL_TAB_MODEL_JNI_BRIDGE_H_
