// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the implementation of the command buffer helper class.

#include "gpu/command_buffer/client/cmd_buffer_helper.h"

#include "base/logging.h"
#include "gpu/command_buffer/common/command_buffer.h"
#include "gpu/command_buffer/common/trace_event.h"

namespace gpu {

CommandBufferHelper::CommandBufferHelper(CommandBuffer* command_buffer)
    : command_buffer_(command_buffer),
      ring_buffer_id_(-1),
      ring_buffer_size_(0),
      entries_(NULL),
      total_entry_count_(0),
      immediate_entry_count_(0),
      token_(0),
      put_(0),
      last_put_sent_(0),
#if defined(CMD_HELPER_PERIODIC_FLUSH_CHECK)
      commands_issued_(0),
#endif
      usable_(true),
      context_lost_(false),
      flush_automatically_(true),
      last_flush_time_(0) {
}

void CommandBufferHelper::SetAutomaticFlushes(bool enabled) {
  flush_automatically_ = enabled;
  CalcImmediateEntries(0);
}

bool CommandBufferHelper::IsContextLost() {
  if (!context_lost_) {
    context_lost_ = error::IsError(command_buffer()->GetLastError());
  }
  return context_lost_;
}

void CommandBufferHelper::CalcImmediateEntries(int waiting_count) {
  DCHECK_GE(waiting_count, 0);

  // Check if usable & allocated.
  if (!usable() || !HaveRingBuffer()) {
    immediate_entry_count_ = 0;
    return;
  }

  // Get maximum safe contiguous entries.
  const int32 curr_get = get_offset();
  if (curr_get > put_) {
    immediate_entry_count_ = curr_get - put_ - 1;
  } else {
    immediate_entry_count_ =
        total_entry_count_ - put_ - (curr_get == 0 ? 1 : 0);
  }

  // Limit entry count to force early flushing.
  if (flush_automatically_) {
    int32 limit =
        total_entry_count_ /
        ((curr_get == last_put_sent_) ? kAutoFlushSmall : kAutoFlushBig);

    int32 pending =
        (put_ + total_entry_count_ - last_put_sent_) % total_entry_count_;

    if (pending > 0 && pending >= limit) {
      // Time to force flush.
      immediate_entry_count_ = 0;
    } else {
      // Limit remaining entries, but not lower than waiting_count entries to
      // prevent deadlock when command size is greater than the flush limit.
      limit -= pending;
      limit = limit < waiting_count ? waiting_count : limit;
      immediate_entry_count_ =
          immediate_entry_count_ > limit ? limit : immediate_entry_count_;
    }
  }
}

bool CommandBufferHelper::AllocateRingBuffer() {
  if (!usable()) {
    return false;
  }

  if (HaveRingBuffer()) {
    return true;
  }

  int32 id = -1;
  Buffer buffer = command_buffer_->CreateTransferBuffer(ring_buffer_size_, &id);
  if (id < 0) {
    ClearUsable();
    return false;
  }

  ring_buffer_ = buffer;
  ring_buffer_id_ = id;
  command_buffer_->SetGetBuffer(id);

  // TODO(gman): Do we really need to call GetState here? We know get & put = 0
  // Also do we need to check state.num_entries?
  CommandBuffer::State state = command_buffer_->GetState();
  entries_ = static_cast<CommandBufferEntry*>(ring_buffer_.ptr);
  int32 num_ring_buffer_entries =
      ring_buffer_size_ / sizeof(CommandBufferEntry);
  if (num_ring_buffer_entries > state.num_entries) {
    ClearUsable();
    return false;
  }

  total_entry_count_ = num_ring_buffer_entries;
  put_ = state.put_offset;
  CalcImmediateEntries(0);
  return true;
}

void CommandBufferHelper::FreeResources() {
  if (HaveRingBuffer()) {
    command_buffer_->DestroyTransferBuffer(ring_buffer_id_);
    ring_buffer_id_ = -1;
    CalcImmediateEntries(0);
  }
}

void CommandBufferHelper::FreeRingBuffer() {
  CHECK((put_ == get_offset()) ||
      error::IsError(command_buffer_->GetLastState().error));
  FreeResources();
}

bool CommandBufferHelper::Initialize(int32 ring_buffer_size) {
  ring_buffer_size_ = ring_buffer_size;
  return AllocateRingBuffer();
}

CommandBufferHelper::~CommandBufferHelper() {
  FreeResources();
}

bool CommandBufferHelper::FlushSync() {
  if (!usable()) {
    return false;
  }

  // Wrap put_ before flush.
  if (put_ == total_entry_count_)
    put_ = 0;

  last_flush_time_ = clock();
  last_put_sent_ = put_;
  CommandBuffer::State state = command_buffer_->FlushSync(put_, get_offset());
  ++flush_generation_;
  CalcImmediateEntries(0);
  return state.error == error::kNoError;
}

void CommandBufferHelper::Flush() {
  // Wrap put_ before flush.
  if (put_ == total_entry_count_)
    put_ = 0;

  if (usable() && last_put_sent_ != put_) {
    last_flush_time_ = clock();
    last_put_sent_ = put_;
    command_buffer_->Flush(put_);
    ++flush_generation_;
    CalcImmediateEntries(0);
  }
}

#if defined(CMD_HELPER_PERIODIC_FLUSH_CHECK)
void CommandBufferHelper::PeriodicFlushCheck() {
  clock_t current_time = clock();
  if (current_time - last_flush_time_ > kPeriodicFlushDelay * CLOCKS_PER_SEC)
    Flush();
}
#endif

// Calls Flush() and then waits until the buffer is empty. Break early if the
// error is set.
bool CommandBufferHelper::Finish() {
  TRACE_EVENT0("gpu", "CommandBufferHelper::Finish");
  if (!usable()) {
    return false;
  }
  // If there is no work just exit.
  if (put_ == get_offset()) {
    return true;
  }
  DCHECK(HaveRingBuffer());
  do {
    // Do not loop forever if the flush fails, meaning the command buffer reader
    // has shutdown.
    if (!FlushSync())
      return false;
  } while (put_ != get_offset());

  return true;
}

// Inserts a new token into the command stream. It uses an increasing value
// scheme so that we don't lose tokens (a token has passed if the current token
// value is higher than that token). Calls Finish() if the token value wraps,
// which will be rare.
int32 CommandBufferHelper::InsertToken() {
  AllocateRingBuffer();
  if (!usable()) {
    return token_;
  }
  DCHECK(HaveRingBuffer());
  // Increment token as 31-bit integer. Negative values are used to signal an
  // error.
  token_ = (token_ + 1) & 0x7FFFFFFF;
  cmd::SetToken* cmd = GetCmdSpace<cmd::SetToken>();
  if (cmd) {
    cmd->Init(token_);
    if (token_ == 0) {
      TRACE_EVENT0("gpu", "CommandBufferHelper::InsertToken(wrapped)");
      // we wrapped
      Finish();
      DCHECK_EQ(token_, last_token_read());
    }
  }
  return token_;
}

// Waits until the current token value is greater or equal to the value passed
// in argument.
void CommandBufferHelper::WaitForToken(int32 token) {
  if (!usable() || !HaveRingBuffer()) {
    return;
  }
  // Return immediately if corresponding InsertToken failed.
  if (token < 0)
    return;
  if (token > token_) return;  // we wrapped
  while (last_token_read() < token) {
    if (get_offset() == put_) {
      LOG(FATAL) << "Empty command buffer while waiting on a token.";
      return;
    }
    // Do not loop forever if the flush fails, meaning the command buffer reader
    // has shutdown.
    if (!FlushSync())
      return;
  }
}

// Waits for available entries, basically waiting until get >= put + count + 1.
// It actually waits for contiguous entries, so it may need to wrap the buffer
// around, adding a noops. Thus this function may change the value of put_. The
// function will return early if an error occurs, in which case the available
// space may not be available.
void CommandBufferHelper::WaitForAvailableEntries(int32 count) {
  AllocateRingBuffer();
  if (!usable()) {
    return;
  }
  DCHECK(HaveRingBuffer());
  DCHECK(count < total_entry_count_);
  if (put_ + count > total_entry_count_) {
    // There's not enough room between the current put and the end of the
    // buffer, so we need to wrap. We will add noops all the way to the end,
    // but we need to make sure get wraps first, actually that get is 1 or
    // more (since put will wrap to 0 after we add the noops).
    DCHECK_LE(1, put_);
    int32 curr_get = get_offset();
    if (curr_get > put_ || curr_get == 0) {
      TRACE_EVENT0("gpu", "CommandBufferHelper::WaitForAvailableEntries");
      while (curr_get > put_ || curr_get == 0) {
        // Do not loop forever if the flush fails, meaning the command buffer
        // reader has shutdown.
        if (!FlushSync())
          return;
        curr_get = get_offset();
      }
    }
    // Insert Noops to fill out the buffer.
    int32 num_entries = total_entry_count_ - put_;
    while (num_entries > 0) {
      int32 num_to_skip = std::min(CommandHeader::kMaxSize, num_entries);
      cmd::Noop::Set(&entries_[put_], num_to_skip);
      put_ += num_to_skip;
      num_entries -= num_to_skip;
    }
    put_ = 0;
  }

  // Try to get 'count' entries without flushing.
  CalcImmediateEntries(count);
  if (immediate_entry_count_ < count) {
    // Try again with a shallow Flush().
    Flush();
    CalcImmediateEntries(count);
    if (immediate_entry_count_ < count) {
      // Buffer is full.  Need to wait for entries.
      TRACE_EVENT0("gpu", "CommandBufferHelper::WaitForAvailableEntries1");
      while (immediate_entry_count_ < count) {
        // Do not loop forever if the flush fails, meaning the command buffer
        // reader has shutdown.
        if (!FlushSync())
          return;
        CalcImmediateEntries(count);
      }
    }
  }
}


}  // namespace gpu
