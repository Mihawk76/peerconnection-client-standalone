/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "p2ptransport.h" // dep 1 // dep 2 // dep 3 // dep loop final //

#include <string>

#include "base64.h" // dep 1 // dep 2 // dep 3 // dep loop final //
#include "common.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep loop final //
#include "stringencode.h" // dep final //
#include "stringutils.h" // dep final //
#include "p2ptransportchannel.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep loop final //

namespace cricket {

P2PTransport::P2PTransport(const std::string& name, PortAllocator* allocator)
    : Transport(name, allocator) {}

P2PTransport::~P2PTransport() {
  DestroyAllChannels();
}

TransportChannelImpl* P2PTransport::CreateTransportChannel(int component) {
  return new P2PTransportChannel(name(), component, this, port_allocator());
}

void P2PTransport::DestroyTransportChannel(TransportChannelImpl* channel) {
  delete channel;
}

}  // namespace cricket
