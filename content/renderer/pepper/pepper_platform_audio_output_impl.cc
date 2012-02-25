// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/pepper/pepper_platform_audio_output_impl.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop_proxy.h"
#include "build/build_config.h"
#include "content/common/child_process.h"
#include "content/common/media/audio_messages.h"
#include "content/renderer/media/audio_hardware.h"
#include "content/renderer/render_thread_impl.h"

PepperPlatformAudioOutputImpl::PepperPlatformAudioOutputImpl()
    : client_(NULL),
      stream_id_(0),
      main_message_loop_proxy_(base::MessageLoopProxy::current()) {
  filter_ = RenderThreadImpl::current()->audio_message_filter();
}

PepperPlatformAudioOutputImpl::~PepperPlatformAudioOutputImpl() {
  // Make sure we have been shut down. Warning: this will usually happen on
  // the I/O thread!
  DCHECK_EQ(0, stream_id_);
  DCHECK(!client_);
}

bool PepperPlatformAudioOutputImpl::Initialize(
    uint32_t sample_rate,
    uint32_t sample_count,
    webkit::ppapi::PluginDelegate::PlatformAudioCommonClient* client) {
  DCHECK(client);
  // Make sure we don't call init more than once.
  DCHECK_EQ(0, stream_id_);

  client_ = client;

  AudioParameters params;
  const uint32_t kMaxSampleCountForLowLatency = 2048;
  // Use the low latency back end if the client request is compatible, and
  // the sample count is low enough to justify using AUDIO_PCM_LOW_LATENCY.
  if (sample_rate == audio_hardware::GetOutputSampleRate() &&
      sample_count <= kMaxSampleCountForLowLatency &&
      sample_count % audio_hardware::GetOutputBufferSize() == 0)
    params.format = AudioParameters::AUDIO_PCM_LOW_LATENCY;
  else
    params.format = AudioParameters::AUDIO_PCM_LINEAR;
  params.channels = 2;
  params.sample_rate = sample_rate;
  params.bits_per_sample = 16;
  params.samples_per_packet = sample_count;

  ChildProcess::current()->io_message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&PepperPlatformAudioOutputImpl::InitializeOnIOThread,
                 this, params));
  return true;
}

bool PepperPlatformAudioOutputImpl::StartPlayback() {
  if (filter_) {
    ChildProcess::current()->io_message_loop()->PostTask(
        FROM_HERE,
        base::Bind(&PepperPlatformAudioOutputImpl::StartPlaybackOnIOThread,
                   this));
    return true;
  }
  return false;
}

bool PepperPlatformAudioOutputImpl::StopPlayback() {
  if (filter_) {
    ChildProcess::current()->io_message_loop()->PostTask(
        FROM_HERE,
        base::Bind(&PepperPlatformAudioOutputImpl::StopPlaybackOnIOThread,
                   this));
    return true;
  }
  return false;
}

void PepperPlatformAudioOutputImpl::ShutDown() {
  // Called on the main thread to stop all audio callbacks. We must only change
  // the client on the main thread, and the delegates from the I/O thread.
  client_ = NULL;
  ChildProcess::current()->io_message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&PepperPlatformAudioOutputImpl::ShutDownOnIOThread, this));
}

void PepperPlatformAudioOutputImpl::InitializeOnIOThread(
    const AudioParameters& params) {
  stream_id_ = filter_->AddDelegate(this);
  filter_->Send(new AudioHostMsg_CreateStream(stream_id_, params));
}

void PepperPlatformAudioOutputImpl::StartPlaybackOnIOThread() {
  if (stream_id_)
    filter_->Send(new AudioHostMsg_PlayStream(stream_id_));
}

void PepperPlatformAudioOutputImpl::StopPlaybackOnIOThread() {
  if (stream_id_)
    filter_->Send(new AudioHostMsg_PauseStream(stream_id_));
}

void PepperPlatformAudioOutputImpl::ShutDownOnIOThread() {
  // Make sure we don't call shutdown more than once.
  if (!stream_id_)
    return;

  filter_->Send(new AudioHostMsg_CloseStream(stream_id_));
  filter_->RemoveDelegate(stream_id_);
  stream_id_ = 0;

  Release();  // Release for the delegate, balances out the reference taken in
              // PepperPluginDelegateImpl::CreateAudio.
}

void PepperPlatformAudioOutputImpl::OnStateChanged(AudioStreamState state) {
}

void PepperPlatformAudioOutputImpl::OnStreamCreated(
    base::SharedMemoryHandle handle,
    base::SyncSocket::Handle socket_handle,
    uint32 length) {
#if defined(OS_WIN)
  DCHECK(handle);
  DCHECK(socket_handle);
#else
  DCHECK_NE(-1, handle.fd);
  DCHECK_NE(-1, socket_handle);
#endif
  DCHECK(length);

  if (base::MessageLoopProxy::current() == main_message_loop_proxy_) {
    // Must dereference the client only on the main thread. Shutdown may have
    // occurred while the request was in-flight, so we need to NULL check.
    if (client_)
      client_->StreamCreated(handle, length, socket_handle);
  } else {
    main_message_loop_proxy_->PostTask(FROM_HERE,
        base::Bind(&PepperPlatformAudioOutputImpl::OnStreamCreated, this,
                   handle, socket_handle, length));
  }
}
