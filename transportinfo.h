/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_P2P_BASE_TRANSPORTINFO_H_
#define WEBRTC_P2P_BASE_TRANSPORTINFO_H_

#include <string>
#include <vector>

#include "candidate.h"  // dep 1 // dep 2 // dep 3 // dep 4 // dep loop final //
#include "constants.h" // dep final //
#include "transportdescription.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep loop final //
#include "helpers.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep 7 // dep 8 // dep loop final //

namespace cricket {

// A TransportInfo is NOT a transport-info message.  It is comparable
// to a "ContentInfo". A transport-infos message is basically just a
// collection of TransportInfos.
struct TransportInfo {
  TransportInfo() {}

  TransportInfo(const std::string& content_name,
                const TransportDescription& description)
      : content_name(content_name),
        description(description) {}

  std::string content_name;
  TransportDescription description;
};

typedef std::vector<TransportInfo> TransportInfos;

}  // namespace cricket

#endif  // WEBRTC_P2P_BASE_TRANSPORTINFO_H_
