/*
 * libjingle
 * Copyright 2009 Google Inc.
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

#ifndef TALK_SESSION_MEDIA_SRTPFILTER_H_
#define TALK_SESSION_MEDIA_SRTPFILTER_H_

#include <list>
#include <map>
#include <string>
#include <vector>

#include "cryptoparams.h" // dep final //
#include "sessiondescription.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep loop final //
#include "basictypes.h" // dep final //
#include "criticalsection.h" // dep final //
#include "scoped_ptr.h" // dep final //
#include "sigslotrepeater.h" // dep final //
#include "sslstreamadapter.h" // dep 1 // dep 2 // dep 3 // dep 4 // dep 5 // dep 6 // dep loop final //

// Forward declaration to avoid pulling in libsrtp headers here
struct srtp_event_data_t;
struct srtp_ctx_t;
struct srtp_policy_t;

namespace cricket {

// Key is 128 bits and salt is 112 bits == 30 bytes. B64 bloat => 40 bytes.
extern const int SRTP_MASTER_KEY_BASE64_LEN;

// Needed for DTLS-SRTP
extern const int SRTP_MASTER_KEY_KEY_LEN;
extern const int SRTP_MASTER_KEY_SALT_LEN;

class SrtpSession;
class SrtpStat;

void EnableSrtpDebugging();
void ShutdownSrtp();

// Class to transform SRTP to/from RTP.
// Initialize by calling SetSend with the local security params, then call
// SetRecv once the remote security params are received. At that point
// Protect/UnprotectRt(c)p can be called to encrypt/decrypt data.
// TODO: Figure out concurrency policy for SrtpFilter.
class SrtpFilter {
 public:
  enum Mode {
    PROTECT,
    UNPROTECT
  };
  enum Error {
    ERROR_NONE,
    ERROR_FAIL,
    ERROR_AUTH,
    ERROR_REPLAY,
  };

  SrtpFilter();
  ~SrtpFilter();

  // Whether the filter is active (i.e. crypto has been properly negotiated).
  bool IsActive() const;

  // Indicates which crypto algorithms and keys were contained in the offer.
  // offer_params should contain a list of available parameters to use, or none,
  // if crypto is not desired. This must be called before SetAnswer.
  bool SetOffer(const std::vector<CryptoParams>& offer_params,
                ContentSource source);
  // Same as SetAnwer. But multiple calls are allowed to SetProvisionalAnswer
  // after a call to SetOffer.
  bool SetProvisionalAnswer(const std::vector<CryptoParams>& answer_params,
                            ContentSource source);
  // Indicates which crypto algorithms and keys were contained in the answer.
  // answer_params should contain the negotiated parameters, which may be none,
  // if crypto was not desired or could not be negotiated (and not required).
  // This must be called after SetOffer. If crypto negotiation completes
  // successfully, this will advance the filter to the active state.
  bool SetAnswer(const std::vector<CryptoParams>& answer_params,
                 ContentSource source);

  // Just set up both sets of keys directly.
  // Used with DTLS-SRTP.
  bool SetRtpParams(int send_cs,
                    const uint8_t* send_key,
                    int send_key_len,
                    int recv_cs,
                    const uint8_t* recv_key,
                    int recv_key_len);
  bool SetRtcpParams(int send_cs,
                     const uint8_t* send_key,
                     int send_key_len,
                     int recv_cs,
                     const uint8_t* recv_key,
                     int recv_key_len);

  // Encrypts/signs an individual RTP/RTCP packet, in-place.
  // If an HMAC is used, this will increase the packet size.
  bool ProtectRtp(void* data, int in_len, int max_len, int* out_len);
  // Overloaded version, outputs packet index.
  bool ProtectRtp(void* data,
                  int in_len,
                  int max_len,
                  int* out_len,
                  int64_t* index);
  bool ProtectRtcp(void* data, int in_len, int max_len, int* out_len);
  // Decrypts/verifies an invidiual RTP/RTCP packet.
  // If an HMAC is used, this will decrease the packet size.
  bool UnprotectRtp(void* data, int in_len, int* out_len);
  bool UnprotectRtcp(void* data, int in_len, int* out_len);

  // Returns rtp auth params from srtp context.
  bool GetRtpAuthParams(uint8_t** key, int* key_len, int* tag_len);

  // Update the silent threshold (in ms) for signaling errors.
  void set_signal_silent_time(uint32_t signal_silent_time_in_ms);

  bool ResetParams();

  sigslot::repeater3<uint32_t, Mode, Error> SignalSrtpError;

 protected:
  bool ExpectOffer(ContentSource source);
  bool StoreParams(const std::vector<CryptoParams>& params,
                   ContentSource source);
  bool ExpectAnswer(ContentSource source);
  bool DoSetAnswer(const std::vector<CryptoParams>& answer_params,
                     ContentSource source,
                     bool final);
  void CreateSrtpSessions();
  bool NegotiateParams(const std::vector<CryptoParams>& answer_params,
                       CryptoParams* selected_params);
  bool ApplyParams(const CryptoParams& send_params,
                   const CryptoParams& recv_params);
  static bool ParseKeyParams(const std::string& params, uint8_t* key, int len);

 private:
  enum State {
    ST_INIT,           // SRTP filter unused.
    ST_SENTOFFER,      // Offer with SRTP parameters sent.
    ST_RECEIVEDOFFER,  // Offer with SRTP parameters received.
    ST_SENTPRANSWER_NO_CRYPTO,  // Sent provisional answer without crypto.
    // Received provisional answer without crypto.
    ST_RECEIVEDPRANSWER_NO_CRYPTO,
    ST_ACTIVE,         // Offer and answer set.
    // SRTP filter is active but new parameters are offered.
    // When the answer is set, the state transitions to ST_ACTIVE or ST_INIT.
    ST_SENTUPDATEDOFFER,
    // SRTP filter is active but new parameters are received.
    // When the answer is set, the state transitions back to ST_ACTIVE.
    ST_RECEIVEDUPDATEDOFFER,
    // SRTP filter is active but the sent answer is only provisional.
    // When the final answer is set, the state transitions to ST_ACTIVE or
    // ST_INIT.
    ST_SENTPRANSWER,
    // SRTP filter is active but the received answer is only provisional.
    // When the final answer is set, the state transitions to ST_ACTIVE or
    // ST_INIT.
    ST_RECEIVEDPRANSWER
  };
  State state_;
  uint32_t signal_silent_time_in_ms_;
  std::vector<CryptoParams> offer_params_;
  rtc::scoped_ptr<SrtpSession> send_session_;
  rtc::scoped_ptr<SrtpSession> recv_session_;
  rtc::scoped_ptr<SrtpSession> send_rtcp_session_;
  rtc::scoped_ptr<SrtpSession> recv_rtcp_session_;
  CryptoParams applied_send_params_;
  CryptoParams applied_recv_params_;
};

// Class that wraps a libSRTP session.
class SrtpSession {
 public:
  SrtpSession();
  ~SrtpSession();

  // Configures the session for sending data using the specified
  // cipher-suite and key. Receiving must be done by a separate session.
  bool SetSend(int cs, const uint8_t* key, int len);
  // Configures the session for receiving data using the specified
  // cipher-suite and key. Sending must be done by a separate session.
  bool SetRecv(int cs, const uint8_t* key, int len);

  // Encrypts/signs an individual RTP/RTCP packet, in-place.
  // If an HMAC is used, this will increase the packet size.
  bool ProtectRtp(void* data, int in_len, int max_len, int* out_len);
  // Overloaded version, outputs packet index.
  bool ProtectRtp(void* data,
                  int in_len,
                  int max_len,
                  int* out_len,
                  int64_t* index);
  bool ProtectRtcp(void* data, int in_len, int max_len, int* out_len);
  // Decrypts/verifies an invidiual RTP/RTCP packet.
  // If an HMAC is used, this will decrease the packet size.
  bool UnprotectRtp(void* data, int in_len, int* out_len);
  bool UnprotectRtcp(void* data, int in_len, int* out_len);

  // Helper method to get authentication params.
  bool GetRtpAuthParams(uint8_t** key, int* key_len, int* tag_len);

  // Update the silent threshold (in ms) for signaling errors.
  void set_signal_silent_time(uint32_t signal_silent_time_in_ms);

  // Calls srtp_shutdown if it's initialized.
  static void Terminate();

  sigslot::repeater3<uint32_t, SrtpFilter::Mode, SrtpFilter::Error>
      SignalSrtpError;

 private:
  bool SetKey(int type, int cs, const uint8_t* key, int len);
    // Returns send stream current packet index from srtp db.
  bool GetSendStreamPacketIndex(void* data, int in_len, int64_t* index);

  static bool Init();
  void HandleEvent(const srtp_event_data_t* ev);
  static void HandleEventThunk(srtp_event_data_t* ev);

  static std::list<SrtpSession*>* sessions();

  srtp_ctx_t* session_;
  int rtp_auth_tag_len_;
  int rtcp_auth_tag_len_;
  rtc::scoped_ptr<SrtpStat> srtp_stat_;
  static bool inited_;
  static rtc::GlobalLockPod lock_;
  int last_send_seq_num_;
  RTC_DISALLOW_COPY_AND_ASSIGN(SrtpSession);
};

// Class that collects failures of SRTP.
class SrtpStat {
 public:
  SrtpStat();

  // Report RTP protection results to the handler.
  void AddProtectRtpResult(uint32_t ssrc, int result);
  // Report RTP unprotection results to the handler.
  void AddUnprotectRtpResult(uint32_t ssrc, int result);
  // Report RTCP protection results to the handler.
  void AddProtectRtcpResult(int result);
  // Report RTCP unprotection results to the handler.
  void AddUnprotectRtcpResult(int result);

  // Get silent time (in ms) for SRTP statistics handler.
  uint32_t signal_silent_time() const { return signal_silent_time_; }
  // Set silent time (in ms) for SRTP statistics handler.
  void set_signal_silent_time(uint32_t signal_silent_time) {
    signal_silent_time_ = signal_silent_time;
  }

  // Sigslot for reporting errors.
  sigslot::signal3<uint32_t, SrtpFilter::Mode, SrtpFilter::Error>
      SignalSrtpError;

 private:
  // For each different ssrc and error, we collect statistics separately.
  struct FailureKey {
    FailureKey()
        : ssrc(0),
          mode(SrtpFilter::PROTECT),
          error(SrtpFilter::ERROR_NONE) {
    }
    FailureKey(uint32_t in_ssrc,
               SrtpFilter::Mode in_mode,
               SrtpFilter::Error in_error)
        : ssrc(in_ssrc), mode(in_mode), error(in_error) {}
    bool operator <(const FailureKey& key) const {
      return
          (ssrc < key.ssrc) ||
          (ssrc == key.ssrc && mode < key.mode) ||
          (ssrc == key.ssrc && mode == key.mode && error < key.error);
    }
    uint32_t ssrc;
    SrtpFilter::Mode mode;
    SrtpFilter::Error error;
  };
  // For tracing conditions for signaling, currently we only use
  // last_signal_time.  Wrap this as a struct so that later on, if we need any
  // other improvements, it will be easier.
  struct FailureStat {
    FailureStat()
        : last_signal_time(0) {
    }
    explicit FailureStat(uint32_t in_last_signal_time)
        : last_signal_time(in_last_signal_time) {}
    void Reset() {
      last_signal_time = 0;
    }
    uint32_t last_signal_time;
  };

  // Inspect SRTP result and signal error if needed.
  void HandleSrtpResult(const FailureKey& key);

  std::map<FailureKey, FailureStat> failures_;
  // Threshold in ms to silent the signaling errors.
  uint32_t signal_silent_time_;

  RTC_DISALLOW_COPY_AND_ASSIGN(SrtpStat);
};

}  // namespace cricket

#endif  // TALK_SESSION_MEDIA_SRTPFILTER_H_
