// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_ENROLLMENT_SCREEN_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_ENROLLMENT_SCREEN_HANDLER_H_

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "chrome/browser/chromeos/login/enrollment/enrollment_screen_actor.h"
#include "chrome/browser/chromeos/login/enrollment/enterprise_enrollment_helper.h"
#include "chrome/browser/chromeos/login/screens/error_screen_actor.h"
#include "chrome/browser/chromeos/login/ui/webui_login_view.h"
#include "chrome/browser/chromeos/policy/enrollment_config.h"
#include "chrome/browser/extensions/signin/scoped_gaia_auth_extension.h"
#include "chrome/browser/ui/webui/chromeos/login/base_screen_handler.h"
#include "chrome/browser/ui/webui/chromeos/login/network_state_informer.h"

namespace chromeos {

class ErrorScreensHistogramHelper;

// WebUIMessageHandler implementation which handles events occurring on the
// page, such as the user pressing the signin button.
class EnrollmentScreenHandler
    : public BaseScreenHandler,
      public EnrollmentScreenActor,
      public NetworkStateInformer::NetworkStateInformerObserver,
      public WebUILoginView::FrameObserver {
 public:
  EnrollmentScreenHandler(
      const scoped_refptr<NetworkStateInformer>& network_state_informer,
      ErrorScreenActor* error_screen_actor);
  virtual ~EnrollmentScreenHandler();

  // Implements WebUIMessageHandler:
  virtual void RegisterMessages() override;

  // Implements EnrollmentScreenActor:
  virtual void SetParameters(Controller* controller,
                             const policy::EnrollmentConfig& config) override;
  virtual void PrepareToShow() override;
  virtual void Show() override;
  virtual void Hide() override;
  virtual void ShowSigninScreen() override;
  virtual void ShowEnrollmentSpinnerScreen() override;
  virtual void ShowAuthError(const GoogleServiceAuthError& error) override;
  virtual void ShowEnrollmentStatus(policy::EnrollmentStatus status) override;
  virtual void ShowOtherError(
      EnterpriseEnrollmentHelper::OtherError error_code) override;

  // Implements BaseScreenHandler:
  virtual void Initialize() override;
  virtual void DeclareLocalizedValues(LocalizedValuesBuilder* builder) override;

  // Implements NetworkStateInformer::NetworkStateInformerObserver
  virtual void UpdateState(ErrorScreenActor::ErrorReason reason) override;

  // Implements WebUILoginView::FrameObserver
  virtual void OnFrameError(const std::string& frame_unique_name) override;

 private:
  // Handlers for WebUI messages.
  void HandleClose(const std::string& reason);
  void HandleCompleteLogin(const std::string& user);
  void HandleRetry();
  void HandleFrameLoadingCompleted(int status);

  void UpdateStateInternal(ErrorScreenActor::ErrorReason reason,
                           bool force_update);
  void SetupAndShowOfflineMessage(NetworkStateInformer::State state,
                                  ErrorScreenActor::ErrorReason reason);
  void HideOfflineMessage(NetworkStateInformer::State state,
                          ErrorScreenActor::ErrorReason reason);

  net::Error frame_error() const { return frame_error_; }
  // Shows a given enrollment step.
  void ShowStep(const char* step);

  // Display the given i18n resource as error message.
  void ShowError(int message_id, bool retry);

  // Display the given string as error message.
  void ShowErrorMessage(const std::string& message, bool retry);

  // Display the given i18n string as a progress message.
  void ShowWorking(int message_id);

  // Shows the screen.
  void DoShow();

  // Returns current visible screen.
  OobeUI::Screen GetCurrentScreen() const;

  // Returns true if current visible screen is the enrollment sign-in page.
  bool IsOnEnrollmentScreen() const;

  // Returns true if current visible screen is the error screen over
  // enrollment sign-in page.
  bool IsEnrollmentScreenHiddenByError() const;

  // Keeps the controller for this actor.
  Controller* controller_;

  bool show_on_init_;

  // The enrollment configuration.
  policy::EnrollmentConfig config_;

  // Whether an enrollment attempt has failed.
  bool enrollment_failed_once_;

  // Latest enrollment frame error.
  net::Error frame_error_;

  // True if screen was not shown yet.
  bool first_show_;

  // Whether we should handle network errors on enrollment screen.
  // True when signin screen step is shown.
  bool observe_network_failure_;

  // Network state informer used to keep signin screen up.
  scoped_refptr<NetworkStateInformer> network_state_informer_;

  ErrorScreenActor* error_screen_actor_;

  scoped_ptr<ErrorScreensHistogramHelper> histogram_helper_;

  // GAIA extension loader.
  scoped_ptr<ScopedGaiaAuthExtension> auth_extension_;

  base::WeakPtrFactory<EnrollmentScreenHandler> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(EnrollmentScreenHandler);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_CHROMEOS_LOGIN_ENROLLMENT_SCREEN_HANDLER_H_
