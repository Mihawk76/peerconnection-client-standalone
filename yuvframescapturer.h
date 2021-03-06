/*
 * libjingle
 * Copyright 2010 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TALK_MEDIA_DEVICES_YUVFRAMESCAPTURER_H_
#define TALK_MEDIA_DEVICES_YUVFRAMESCAPTURER_H_

#include <string>
#include <vector>

#include "videocapturer.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 //
#include "yuvframegenerator.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep 7 // dep 8 // dep 9 //
#include "stream.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 //
#include "stringutils.h" // dep 1 // dep final //


namespace rtc {
class FileStream;
}

namespace cricket {


// Simulated video capturer that periodically reads frames from a file.
class YuvFramesCapturer : public VideoCapturer {
 public:
  YuvFramesCapturer();
  YuvFramesCapturer(int width, int height);
  virtual ~YuvFramesCapturer();

  static const char* kYuvFrameDeviceName;
  static Device CreateYuvFramesCapturerDevice() {
    std::stringstream id;
    id << kYuvFrameDeviceName;
    return Device(id.str(), id.str());
  }
  static bool IsYuvFramesCapturerDevice(const Device& device) {
    return rtc::starts_with(device.id.c_str(), kYuvFrameDeviceName);
  }

  void Init();
  // Override virtual methods of parent class VideoCapturer.
  virtual CaptureState Start(const VideoFormat& capture_format);
  virtual void Stop();
  virtual bool IsRunning();
  virtual bool IsScreencast() const { return false; }

 protected:
  // Override virtual methods of parent class VideoCapturer.
  virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs);

  // Read a frame and determine how long to wait for the next frame.
  void ReadFrame(bool first_frame);

 private:
  class YuvFramesThread;  // Forward declaration, defined in .cc.

  YuvFrameGenerator* frame_generator_;
  CapturedFrame captured_frame_;
  YuvFramesThread* frames_generator_thread;
  int width_;
  int height_;
  uint32_t frame_data_size_;
  uint32_t frame_index_;

  int64_t barcode_reference_timestamp_millis_;
  int32_t barcode_interval_;
  int32_t GetBarcodeValue();

  RTC_DISALLOW_COPY_AND_ASSIGN(YuvFramesCapturer);
};

}  // namespace cricket

#endif  // TALK_MEDIA_DEVICES_YUVFRAMESCAPTURER_H_
