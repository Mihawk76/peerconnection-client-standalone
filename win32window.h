/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_BASE_WIN32WINDOW_H_
#define WEBRTC_BASE_WIN32WINDOW_H_

#if defined(WEBRTC_WIN)

#include "win32.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep 7 // dep loop final //

namespace rtc {

///////////////////////////////////////////////////////////////////////////////
// Win32Window
///////////////////////////////////////////////////////////////////////////////

class Win32Window {
 public:
  Win32Window();
  virtual ~Win32Window();

  HWND handle() const { return wnd_; }

  bool Create(HWND parent, const wchar_t* title, DWORD style, DWORD exstyle,
              int x, int y, int cx, int cy);
  void Destroy();

  // Call this when your DLL unloads.
  static void Shutdown();

 protected:
  virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam,
                         LRESULT& result);

  virtual bool OnClose() { return true; }
  virtual void OnNcDestroy() { }

 private:
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam);

  HWND wnd_;
  static HINSTANCE instance_;
  static ATOM window_class_;
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace rtc

#endif  // WEBRTC_WIN 

#endif  // WEBRTC_BASE_WIN32WINDOW_H_
