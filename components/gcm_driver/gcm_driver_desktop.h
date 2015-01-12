// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_GCM_DRIVER_GCM_DRIVER_DESKTOP_H_
#define COMPONENTS_GCM_DRIVER_GCM_DRIVER_DESKTOP_H_

#include <map>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "components/gcm_driver/gcm_channel_status_syncer.h"
#include "components/gcm_driver/gcm_client.h"
#include "components/gcm_driver/gcm_connection_observer.h"
#include "components/gcm_driver/gcm_driver.h"

class PrefService;

namespace base {
class FilePath;
class SequencedTaskRunner;
}

namespace extensions {
class ExtensionGCMAppHandlerTest;
}

namespace net {
class URLRequestContextGetter;
}

namespace gcm {

class GCMAccountMapper;
class GCMAppHandler;
class GCMClientFactory;
class GCMDelayedTaskController;

// GCMDriver implementation for desktop and Chrome OS, using GCMClient.
class GCMDriverDesktop : public GCMDriver {
 public:
  GCMDriverDesktop(
      scoped_ptr<GCMClientFactory> gcm_client_factory,
      const GCMClient::ChromeBuildInfo& chrome_build_info,
      const std::string& channel_status_request_url,
      const std::string& user_agent,
      PrefService* prefs,
      const base::FilePath& store_path,
      const scoped_refptr<net::URLRequestContextGetter>& request_context,
      const scoped_refptr<base::SequencedTaskRunner>& ui_thread,
      const scoped_refptr<base::SequencedTaskRunner>& io_thread,
      const scoped_refptr<base::SequencedTaskRunner>& blocking_task_runner);
  ~GCMDriverDesktop() override;

  // GCMDriver overrides:
  void Shutdown() override;
  void OnSignedIn() override;
  void OnSignedOut() override;
  void AddAppHandler(const std::string& app_id,
                     GCMAppHandler* handler) override;
  void RemoveAppHandler(const std::string& app_id) override;
  void AddConnectionObserver(GCMConnectionObserver* observer) override;
  void RemoveConnectionObserver(GCMConnectionObserver* observer) override;
  void Enable() override;
  void Disable() override;
  GCMClient* GetGCMClientForTesting() const override;
  bool IsStarted() const override;
  bool IsConnected() const override;
  void GetGCMStatistics(const GetGCMStatisticsCallback& callback,
                        bool clear_logs) override;
  void SetGCMRecording(const GetGCMStatisticsCallback& callback,
                       bool recording) override;
  void SetAccountTokens(
      const std::vector<GCMClient::AccountTokenInfo>& account_tokens) override;
  void UpdateAccountMapping(const AccountMapping& account_mapping) override;
  void RemoveAccountMapping(const std::string& account_id) override;
  base::Time GetLastTokenFetchTime() override;
  void SetLastTokenFetchTime(const base::Time& time) override;
  void WakeFromSuspendForHeartbeat(bool wake) override;

  // Exposed for testing purpose.
  bool gcm_enabled() const { return gcm_enabled_; }
  GCMChannelStatusSyncer* gcm_channel_status_syncer_for_testing() {
    return gcm_channel_status_syncer_.get();
  }

 protected:
  // GCMDriver implementation:
  GCMClient::Result EnsureStarted() override;
  void RegisterImpl(const std::string& app_id,
                    const std::vector<std::string>& sender_ids) override;
  void UnregisterImpl(const std::string& app_id) override;
  void SendImpl(const std::string& app_id,
                const std::string& receiver_id,
                const GCMClient::OutgoingMessage& message) override;

 private:
  class IOWorker;

  //  Stops the GCM service. It can be restarted by calling EnsureStarted again.
  void Stop();

  // Remove cached data when GCM service is stopped.
  void RemoveCachedData();

  void DoRegister(const std::string& app_id,
                  const std::vector<std::string>& sender_ids);
  void DoUnregister(const std::string& app_id);
  void DoSend(const std::string& app_id,
              const std::string& receiver_id,
              const GCMClient::OutgoingMessage& message);

  // Callbacks posted from IO thread to UI thread.
  void MessageReceived(const std::string& app_id,
                       const GCMClient::IncomingMessage& message);
  void MessagesDeleted(const std::string& app_id);
  void MessageSendError(const std::string& app_id,
                        const GCMClient::SendErrorDetails& send_error_details);
  void SendAcknowledged(const std::string& app_id,
                        const std::string& message_id);
  void GCMClientReady(const std::vector<AccountMapping>& account_mappings,
                      const base::Time& last_token_fetch_time);
  void OnConnected(const net::IPEndPoint& ip_endpoint);
  void OnDisconnected();

  void GetGCMStatisticsFinished(const GCMClient::GCMStatistics& stats);

  scoped_ptr<GCMChannelStatusSyncer> gcm_channel_status_syncer_;

  // Flag to indicate whether the user is signed in to a GAIA account.
  // TODO(jianli): To be removed when sign-in enforcement is dropped.
  bool signed_in_;

  // Flag to indicate if GCM is started.
  bool gcm_started_;

  // Flag to indicate if GCM is enabled.
  // TODO(jianli): Removed when we switch completely to support all users.
  bool gcm_enabled_;

  // Flag to indicate the last known state of the GCM client. Because this
  // flag lives on the UI thread, while the GCM client lives on the IO thread,
  // it may be out of date while connection changes are happening.
  bool connected_;

  // List of observers to notify when connection state changes.
  ObserverList<GCMConnectionObserver, false> connection_observer_list_;

  // Account mapper. Only works when user is signed in.
  scoped_ptr<GCMAccountMapper> account_mapper_;

  // Time of last token fetching.
  base::Time last_token_fetch_time_;

  scoped_refptr<base::SequencedTaskRunner> ui_thread_;
  scoped_refptr<base::SequencedTaskRunner> io_thread_;

  scoped_ptr<GCMDelayedTaskController> delayed_task_controller_;

  // Whether the HeartbeatManager should try to wake the system from suspend for
  // sending heartbeat messages.
  bool wake_from_suspend_enabled_;

  // For all the work occurring on the IO thread. Must be destroyed on the IO
  // thread.
  scoped_ptr<IOWorker> io_worker_;

  // Callback for GetGCMStatistics.
  GetGCMStatisticsCallback request_gcm_statistics_callback_;

  // Used to pass a weak pointer to the IO worker.
  base::WeakPtrFactory<GCMDriverDesktop> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(GCMDriverDesktop);
};

}  // namespace gcm

#endif  // COMPONENTS_GCM_DRIVER_GCM_DRIVER_DESKTOP_H_
