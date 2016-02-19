/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_BASE_NETWORKMONITOR_H_
#define WEBRTC_BASE_NETWORKMONITOR_H_

#include "logging.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep 7 // dep loop final //
#include "scoped_ptr.h" // dep final //
#include "sigslot.h" // dep final //
#include "thread.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep loop final //

namespace rtc {
/*
 * Receives network-change events via |OnNetworksChanged| and signals the
 * networks changed event.
 *
 * Threading consideration:
 * It is expected that all upstream operations (from native to Java) are
 * performed from the worker thread. This includes creating, starting and
 * stopping the monitor. This avoids the potential race condition when creating
 * the singleton Java NetworkMonitor class. Downstream operations can be from
 * any thread, but this class will forward all the downstream operations onto
 * the worker thread.
 *
 * Memory consideration:
 * NetworkMonitor is owned by the caller (NetworkManager). The global network
 * monitor factory is owned by the factory itself but needs to be released from
 * the factory creator.
 */
// Generic network monitor interface. It starts and stops monitoring network
// changes, and fires the SignalNetworksChanged event when networks change.
class NetworkMonitorInterface {
 public:
  NetworkMonitorInterface();
  virtual ~NetworkMonitorInterface();

  sigslot::signal0<> SignalNetworksChanged;

  virtual void Start() = 0;
  virtual void Stop() = 0;

  // Implementations should call this method on the base when networks change,
  // and the base will fire SignalNetworksChanged on the right thread.
  virtual void OnNetworksChanged() = 0;
};

class NetworkMonitorBase : public NetworkMonitorInterface,
                           public MessageHandler,
                           public sigslot::has_slots<> {
 public:
  NetworkMonitorBase();
  ~NetworkMonitorBase() override;

  void OnNetworksChanged() override;

  void OnMessage(Message* msg) override;

 private:
  Thread* thread_;
};

/*
 * NetworkMonitorFactory creates NetworkMonitors.
 */
class NetworkMonitorFactory {
 public:
  // This is not thread-safe; it should be called once (or once per audio/video
  // call) during the call initialization.
  static void SetFactory(NetworkMonitorFactory* factory);

  static void ReleaseFactory(NetworkMonitorFactory* factory);
  static NetworkMonitorFactory* GetFactory();

  virtual NetworkMonitorInterface* CreateNetworkMonitor() = 0;

  virtual ~NetworkMonitorFactory();

 protected:
  NetworkMonitorFactory();
};

}  // namespace rtc

#endif  // WEBRTC_BASE_NETWORKMONITOR_H_
