/*
 *  Copyright 2010 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_BASE_WINDOWPICKERFACTORY_H_
#define WEBRTC_BASE_WINDOWPICKERFACTORY_H_

#if defined(WEBRTC_WIN)
#include "win32windowpicker.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep loop final //
#elif defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)
#include "macutils.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep 7 // dep 8 // dep loop final //
#include "macwindowpicker.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep 7 // dep 8 // dep loop final //
#elif defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID) && defined(HAVE_X11)
#include "x11windowpicker.h"// dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep 7 // dep 8 // dep loop final //
#endif

#include "windowpicker.h" // dep 1 // dep 2 // dep 3 // dep final //

namespace rtc {

class WindowPickerFactory {
 public:
  virtual ~WindowPickerFactory() {}

  // Instance method for dependency injection.
  virtual WindowPicker* Create() {
    return CreateWindowPicker();
  }

  static WindowPicker* CreateWindowPicker() {
#if defined(WEBRTC_WIN)
    return new Win32WindowPicker();
#elif defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)
    return new MacWindowPicker();
#elif defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID) && defined(HAVE_X11)
    return new X11WindowPicker();
#else
    return NULL;
#endif
  }

  static bool IsSupported() {
#if defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)
    return GetOSVersionName() >= kMacOSLeopard;
#else
    return true;
#endif
  }
};

}  // namespace rtc

#endif  // WEBRTC_BASE_WINDOWPICKERFACTORY_H_
