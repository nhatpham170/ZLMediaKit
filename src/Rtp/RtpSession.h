﻿/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_RTPSESSION_H
#define ZLMEDIAKIT_RTPSESSION_H

#if defined(ENABLE_RTPPROXY)

#include "Network/Session.h"
#include "RtpProcess.h"
#include "RtpSplitter.h"
#include "Util/TimeTicker.h"
#include <JT1078Package.h>
#include <Protocol/JT1078/JT1078Process.h>

namespace mediakit {

class RtpSession
    : public toolkit::Session
    , public RtpSplitter {
public:
    static const std::string kVhost;
    static const std::string kApp;
    static const std::string kStreamID;
    static const std::string kSSRC;
    static const std::string kOnlyTrack;
    static const std::string kUdpRecvBuffer;

    RtpSession(const toolkit::Socket::Ptr &sock);
    ~RtpSession() override;
    void AnalysisJT1078(std::string strHex);
    void AnalysisJT1078264(std::string strHex);
    void AnalysisJT1078264New(std::string strHex);
    void onRecv(const toolkit::Buffer::Ptr &) override;
    void onError(const toolkit::SockException &err) override;
    void onManager() override;
    void setParams(toolkit::mINI &ini);
    void attachServer(const toolkit::Server &server) override;
    void setRtpProcess(RtpProcess::Ptr process);

protected:
    // 收到rtp回调  [AUTO-TRANSLATED:446b2cda]
    // Received RTP callback
    void onRtpPacket(const char *data, size_t len) override;
    // RtpSplitter override
    const char *onSearchPacketTail(const char *data, size_t len) override;
    // 搜寻SSRC  [AUTO-TRANSLATED:2cfec2e1]
    // Search for SSRC
    const char *searchBySSRC(const char *data, size_t len);
    // 搜寻PS包里的关键帧标头  [AUTO-TRANSLATED:d8e88339]
    // Search for keyframe header in PS packet
    const char *searchByPsHeaderFlag(const char *data, size_t len);

private:
    bool _is_udp = false;
    bool _search_rtp = false;
    bool _search_rtp_finished = false;
    bool _emit_detach = false;
    int _only_track = 0;
    std::string _protocol;
    
    uint32_t _ssrc = 0;
    toolkit::Ticker _ticker;
    MediaTuple _tuple;
    struct sockaddr_storage _addr;
    RtpProcess::Ptr _process;
    // string dataBuffer;
    std::string _reverse = "";
    uint32_t _time = 0;
    int _lastFrameInterval = 0;
    int _fps = 15;
    int _fps_temp = 0;
    uint64_t _firstTime = 0;
    uint32_t _sqe = 0;
    int _lastType;
    std::string _imei = "";
    std::string _ssrcStr = "";
    std::string _channel = "";
    mediakit::JT1078Package *_lastPackage;
    mediakit::JT1078Process * _jt1078_process;
};

} // namespace mediakit
#endif // defined(ENABLE_RTPPROXY)
#endif // ZLMEDIAKIT_RTPSESSION_H
