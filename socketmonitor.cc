/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "socketmonitor.h" // dep 1 // dep 2 // dep 3 // dep loop final //

#include "common.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep loop final //

namespace cricket {

enum {
  MSG_MONITOR_POLL,
  MSG_MONITOR_START,
  MSG_MONITOR_STOP,
  MSG_MONITOR_SIGNAL
};

ConnectionMonitor::ConnectionMonitor(ConnectionStatsGetter* stats_getter,
                                     rtc::Thread* worker_thread,
                                     rtc::Thread* monitoring_thread) {
  stats_getter_ = stats_getter;
  worker_thread_ = worker_thread;
  monitoring_thread_ = monitoring_thread;
  monitoring_ = false;
}

ConnectionMonitor::~ConnectionMonitor() {
  worker_thread_->Clear(this);
  monitoring_thread_->Clear(this);
}

void ConnectionMonitor::Start(int milliseconds) {
  rate_ = milliseconds;
  if (rate_ < 250)
    rate_ = 250;
  worker_thread_->Post(this, MSG_MONITOR_START);
}

void ConnectionMonitor::Stop() {
  worker_thread_->Post(this, MSG_MONITOR_STOP);
}

void ConnectionMonitor::OnMessage(rtc::Message *message) {
  rtc::CritScope cs(&crit_);
  switch (message->message_id) {
    case MSG_MONITOR_START:
      ASSERT(rtc::Thread::Current() == worker_thread_);
      if (!monitoring_) {
        monitoring_ = true;
        PollConnectionStats_w();
      }
      break;

    case MSG_MONITOR_STOP:
      ASSERT(rtc::Thread::Current() == worker_thread_);
      if (monitoring_) {
        monitoring_ = false;
        worker_thread_->Clear(this);
      }
      break;

    case MSG_MONITOR_POLL:
      ASSERT(rtc::Thread::Current() == worker_thread_);
      PollConnectionStats_w();
      break;

    case MSG_MONITOR_SIGNAL: {
      ASSERT(rtc::Thread::Current() == monitoring_thread_);
      std::vector<ConnectionInfo> infos = connection_infos_;
      crit_.Leave();
      SignalUpdate(this, infos);
      crit_.Enter();
      break;
    }
  }
}

void ConnectionMonitor::PollConnectionStats_w() {
  ASSERT(rtc::Thread::Current() == worker_thread_);
  rtc::CritScope cs(&crit_);

  // Gather connection infos
  stats_getter_->GetConnectionStats(&connection_infos_);

  // Signal the monitoring thread, start another poll timer
  monitoring_thread_->Post(this, MSG_MONITOR_SIGNAL);
  worker_thread_->PostDelayed(rate_, this, MSG_MONITOR_POLL);
}

}  // namespace cricket
