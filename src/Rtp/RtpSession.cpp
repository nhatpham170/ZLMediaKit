/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#if defined(ENABLE_RTPPROXY)
#include "RtpSession.h"
#include "Common/config.h"
#include "Network/TcpServer.h"
#include "RtpProcess.h"
#include "Rtsp/RtpReceiver.h"
#include "Rtsp/Rtsp.h"
#include <chrono>
#include <iomanip>
#include <src/JT1078Package.h>

using namespace std;
using namespace toolkit;

namespace mediakit {

const string RtpSession::kVhost = "vhost";
const string RtpSession::kApp = "app";
const string RtpSession::kStreamID = "stream_id";
const string RtpSession::kSSRC = "ssrc";
const string RtpSession::kOnlyTrack = "only_track";
const string RtpSession::kUdpRecvBuffer = "udp_recv_socket_buffer";

void RtpSession::attachServer(const Server &server) {
    setParams(const_cast<Server &>(server));
}

void RtpSession::setParams(mINI &ini) {
    _tuple.vhost = ini[kVhost];
    _tuple.app = ini[kApp];
    _tuple.stream = ini[kStreamID];
    _ssrc = ini[kSSRC];
    _only_track = ini[kOnlyTrack];
    int udp_socket_buffer = ini[kUdpRecvBuffer];
    if (_is_udp) {
        // 设置udp socket读缓存  [AUTO-TRANSLATED:80cfb6e3]
        // Set udp socket read buffer
        SockUtil::setRecvBuf(getSock()->rawFD(), (udp_socket_buffer > 0) ? udp_socket_buffer : (4 * 1024 * 1024));
    }
}

RtpSession::RtpSession(const Socket::Ptr &sock)
    : Session(sock) {
    socklen_t addr_len = sizeof(_addr);
    getpeername(sock->rawFD(), (struct sockaddr *)&_addr, &addr_len);
    _is_udp = sock->sockType() == SockNum::Sock_UDP;
}

RtpSession::~RtpSession() = default;

std::string hexStr(const uint8_t *data, int len) {
    std::stringstream ss;
    ss << std::hex;

    for (int i(0); i < len; ++i)
        ss << std::setw(2) << std::setfill('0') << (int)data[i];

    return ss.str();
}
struct RtpHeader2 {
    uint8_t csrcLen : 4; // CSRC计数器，占4位，指示CSRC 标识符的个数。
    uint8_t extension : 1; // 占1位，如果X=1，则在RTP报头后跟有一个扩展报头。
    uint8_t padding : 1; // 填充标志，占1位，如果P=1，则在该报文的尾部填充一个或多个额外的八位组，它们不是有效载荷的一部分。
    uint8_t version : 2; // RTP协议的版本号，占2位，当前协议版本号为2。
    uint8_t payloadType : 7; // 有效载荷类型，占7位，用于说明RTP报文中有效载荷的类型，如GSM音频、JPEM图像等。
    uint8_t marker : 1; // 标记，占1位，不同的有效载荷有不同的含义，对于视频，标记一帧的结束；对于音频，标记会话的开始。
    uint16_t seq; // 占16位，用于标识发送者所发送的RTP报文的序列号，每发送一个报文，序列号增1。接收者通过序列号来检测报文丢失情况，重新排序报文，恢复数据。
    uint32_t timestamp; // 占32位，时戳反映了该RTP报文的第一个八位组的采样时刻。接收者使用时戳来计算延迟和延迟抖动，并进行同步控制。
    uint32_t ssrc; // 占32位，用于标识同步信源。该标识符是随机选择的，参加同一视频会议的两个同步信源不能有相同的SSRC。
};

struct RtpPacket2 {
    struct RtpHeader2 rtpHeader;
    uint8_t payload[0];
};

static inline int startCode3(char *buf) {
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

static inline int startCode4(char *buf) {
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return 1;
    else
        return 0;
}

void writeFile(string msg, string fileName) {
    std::ofstream out(fileName, std::ios::app);
    out << msg;
}
void writeFileName(string msg, string fileName) {
    std::ofstream out(fileName, std::ios::app);
    out << msg + '\n';
}

void writeFileChar(char *msg, int count, string fileName) {
    std::ofstream out(fileName, std::ios::app);
    out.write(msg, count);
}

void RtpSession::AnalysisJT1078(std::string strHex) {

    if (true) {
        std::stringstream ss;
        // writeFileName(strHex, "jt1078_264.txt");
        JT1078Package *package = new JT1078Package();
        int index = 0;
        char *p;
        std::string logo = strHex.substr(index, 8);
        index += 8;
        package->header = strHex.substr(index, 4);
        index += 4;
        package->sqe = strHex.substr(index, 4);
        index += 4;
        package->imei = strHex.substr(index, 12);
        index += 12;
        package->channel = strHex.substr(index, 2);
        index += 2;
        std::string lab = strHex.substr(index, 2);
        int label3 = static_cast<int>(stoul(strHex.substr(index, 2), nullptr, 16) & 0xFF);
        int subPackageType = label3 & 0x0f;
        int dataType = label3 >> 4;
        uint64_t time = 0;
        index += 2;

        if (dataType != JT1078Package::DataType::TransparentData) {
            index += 8; // skip 4 byte time
            time = strtol(strHex.substr(index, 8).c_str(), &p, 16);
            index += 8;
            /* if (_firstTime == 0) {
                 _firstTime = time;
             }*/
            // package->timestamp = time;
        }

        if (dataType != JT1078Package::DataType::TransparentData && dataType != JT1078Package::DataType::Audio) {
            // std::string frameInterval = strHex.substr(index, 4);
            package->frameInterval = strtol(strHex.substr(index, 4).c_str(), nullptr, 16);
            index += 4;
            // std::string iframeInterval = strHex.substr(index, 4);
            package->iframeInterval = strtol(strHex.substr(index, 4).c_str(), nullptr, 16);
            index += 4;
        }
        /*if (dataType == JT1078Package::DataType::Audio) {
            return;
        }*/
        package->length = strtol(strHex.substr(index, 4).c_str(), &p, 16);
        index += 4;
        package->body = strHex.substr(index);
        writeFile(package->body, _imei + "_" + _channel + "_h265.txt");
        bool checkPush = false;
        switch (subPackageType) {
            case JT1078Package::SubPackageType::FirstPacket: _lastPackage = package; break;
            case JT1078Package::SubPackageType::SubPacket: _lastPackage->body += package->body; break;
            case JT1078Package::SubPackageType::LastPacket:
                _lastPackage->body += package->body;
                checkPush = true;
                break;
            case JT1078Package::SubPackageType::Fixed:
                _lastPackage = package;
                checkPush = true;
                break;
            default: break;
        }
        if (checkPush && _sqe == 0) {
            InfoP(this) << "Push: " + _tuple.stream;
        }
        if (checkPush) {
            // find nalu
            vector<string> nalus;
            std::string data = _lastPackage->body;
            int dataSize = data.length();
            int seek = 0;
            int start = -1;
            vector<char> arrCharHex;
            int lastType = 0;
            int rtpIndex = 0;
            while (rtpIndex < data.size()) {
                int val = stoi(data.substr(rtpIndex, 2), nullptr, 0x10);
                arrCharHex.push_back((char)val);
                rtpIndex += 2;
            }
            /* writeFileChar(arrCharHex.data(), arrCharHex.size(), "stream.h265");
             writeFileName(data, "stream.h2652");*/

            rtpIndex = 0;
            while (seek < dataSize) {
                int type = 0;
                if (start >= 0 && seek - start < 6) {
                    seek += 2;
                    continue;
                }
                int remaining = dataSize - seek;
                if (remaining >= 8 && data.substr(seek, 8) == "00000001") {
                    // start 4
                    type = 4;
                    lastType = 4;
                }
                // else if (remaining >= 6 && data.substr(seek, 6) == "000001") {
                //     // start 3
                //     type = 3;
                //     lastType = 3;
                // }
                if (type != 0 || remaining <= 2) {
                    if (start >= 0) {
                        int lng = seek - start;
                        if (remaining <= 2) {
                            lng += 2;
                        }
                        std::string nalu = data.substr(start, lng);
                        nalus.push_back(nalu.substr(lastType * 2));
                    }
                    start = seek;
                }
                seek += 2;
            }
            const int MAX_PACKET = 2800;
            // build rtp package
            int toalNalu = nalus.size();
            vector<string> listRtp;
            std::string ssrc = _ssrcStr;
            std::string timeHex = "";
            std::stringstream timeSS;
            timeSS << std::setw(8) << std::setfill('0') << std::hex << _time;
            timeHex = timeSS.str();

            // std::string headerRtp = "8062";
            std::string headerRtp = "8063";
            bool checkIframe = false;
            for (size_t i = 0; i < toalNalu; i++) {
                std::string nalu = nalus[i];
                // write file

                // build rtp
                int naluTypeInt = strtol(nalu.substr(0, 2).c_str(), &p, 16);

                int lengthNalu = nalu.size();

                if (lengthNalu <= MAX_PACKET) {
                    std::stringstream sqeSS;
                    sqeSS << std::setw(4) << std::setfill('0') << std::hex << _sqe;
                    std::string rtp = "";
                    rtp += "80E3";
                    rtp += sqeSS.str(); // 0x0000 - sqe
                    rtp += timeHex; // 0x00000000 - time
                    rtp += ssrc; // 0x00000000 - ssrc
                    rtp += nalu;
                    listRtp.push_back(rtp);
                    _sqe += 1;
                } else {
                    int seekNalu = 0;
                    int indexBulk = 0;
                    int naluType = (naluTypeInt & 0x7E) >> 1;
                    int subNaluType = strtol(nalu.substr(2, 2).c_str(), &p, 16);
                    while (seekNalu < lengthNalu) {
                        int naluRemain = lengthNalu - seekNalu;
                        std::stringstream sqeSS;
                        sqeSS << std::setw(4) << std::setfill('0') << std::hex << _sqe;
                        std::string rtp = "";
                        /*rtp += headerRtp;*/
                        rtp += sqeSS.str(); // 0x0000 - sqe
                        rtp += timeHex; // 0x00000000 - time
                        rtp += ssrc; // 0x00000000 - ssrc
                        int byte0 = (naluTypeInt & 0x81) | (49 << 1);
                        int byte1 = subNaluType;
                        int byte2 = naluType;
                        seekNalu += 4;

                        if (indexBulk == 0) {
                            // first
                            byte2 |= 0x80;
                            rtp = "8063" + rtp;
                        } else if (naluRemain <= MAX_PACKET) {
                            // end
                            byte2 |= 0x40;
                            // marker = 1;
                            rtp = "80E3" + rtp;
                        } else {
                            rtp = "8063" + rtp;
                        }
                        std::stringstream byte0Str;
                        byte0Str << (byte0 < 16 ? "0" : "") << std::hex << byte0;
                        rtp += byte0Str.str();
                        std::stringstream byte1Str;
                        byte1Str << (byte1 < 16 ? "0" : "") << std::hex << byte1;
                        rtp += byte1Str.str();
                        std::stringstream byte2Str;
                        byte2Str << (byte2 < 16 ? "0" : "") << std::hex << byte2;
                        rtp += byte2Str.str();
                        // add frame
                        if (naluRemain > MAX_PACKET) {
                            rtp += nalu.substr(seekNalu, MAX_PACKET);
                        } else {
                            rtp += nalu.substr(seekNalu, naluRemain);
                        }
                        seekNalu += MAX_PACKET;
                        _sqe += 1;
                        indexBulk += 1;
                        listRtp.push_back(rtp);
                    }
                    checkIframe = true;
                }
            }

            // pusb rtp
            for (size_t i = 0; i < listRtp.size(); i++) {
                std::string rtpHex = listRtp[i];
                int rtpLength = rtpHex.length();
                vector<char> arrCharHex;
                //// check JT1078
                int rtpIndex = 0;
                while (rtpIndex < rtpLength) {
                    int val = stoi(rtpHex.substr(rtpIndex, 2), nullptr, 0x10);
                    arrCharHex.push_back((char)val);
                    rtpIndex += 2;
                }
                writeFileName(rtpHex, "jt1078_rtp.txt");
                onRtpPacket(arrCharHex.data(), arrCharHex.size());
            }
            /*if (package->iframeInterval > 0) {
                int delta = 0;
                delta = (double)(package->iframeInterval / 10);
                _time += (int)(delta > 0 ? 90000 / delta : 90000 / 10);
            } else {
                _time += (int)(90000 / 10);
            }*/
            /*_time += (int)(90000 / 15);*/
            /*    if (checkIframe) {
                    _time += (int)(90000 / 15);
                }*/
            auto now = chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            uint64_t milliseconds = chrono::duration_cast<chrono::milliseconds>(duration).count();
            if (_firstTime == 0) {
                _firstTime = milliseconds;
            }
            _time = (uint32_t)((milliseconds - _firstTime) * 90);
            //_time += (int)(90000 / 15);
            // if (time > 0) {
            //     //_time = (int)((time - _firstTime) * 90); // for bsj
            //     _time = (int)((milliseconds - _firstTime) * 90);
            // }
            _lastPackage->body = "";
        }

        return;
    }
}
#pragma region AnalysisJT1078264
//
void RtpSession::AnalysisJT1078264(std::string strHex) {

    if (true) {
        std::stringstream ss;
        // writeFileName(strHex, "jt1078_264.txt");
        JT1078Package *package = new JT1078Package();
        int index = 0;
        char *p;
        std::string logo = strHex.substr(index, 8);
        index += 8;
        package->header = strHex.substr(index, 4);
        index += 4;
        package->sqe = strHex.substr(index, 4);
        index += 4;
        package->imei = strHex.substr(index, 12);
        index += 12;
        package->channel = strHex.substr(index, 2);
        index += 2;
        std::string lab = strHex.substr(index, 2);
        int label3 = static_cast<int>(stoul(strHex.substr(index, 2), nullptr, 16) & 0xFF);
        int subPackageType = label3 & 0x0f;
        int dataType = label3 >> 4;
        uint64_t time = 0;
        index += 2;

        if (dataType != JT1078Package::DataType::TransparentData) {
            index += 8; // skip 4 byte time
            time = strtol(strHex.substr(index, 8).c_str(), &p, 16);
            index += 8;
        }

        if (dataType != JT1078Package::DataType::TransparentData && dataType != JT1078Package::DataType::Audio) {
            package->frameInterval = strtol(strHex.substr(index, 4).c_str(), nullptr, 16);
            index += 4;
            package->iframeInterval = strtol(strHex.substr(index, 4).c_str(), nullptr, 16);
            index += 4;
        }
        package->dataType = dataType;
        package->length = strtol(strHex.substr(index, 4).c_str(), &p, 16);
        index += 4;
        package->body = strHex.substr(index);
        bool checkPush = false;
        switch (subPackageType) {
            case JT1078Package::SubPackageType::FirstPacket: _lastPackage = package; break;
            case JT1078Package::SubPackageType::SubPacket: _lastPackage->body += package->body; break;
            case JT1078Package::SubPackageType::LastPacket:
                _lastPackage->body += package->body;
                _lastPackage->iframeInterval = package->iframeInterval;
                _lastPackage->frameInterval = package->frameInterval;
                checkPush = true;
                break;
            case JT1078Package::SubPackageType::Fixed:
                _lastPackage = package;
                checkPush = true;
                break;
            default: break;
        }
        if (checkPush && _sqe == 0) {
            InfoP(this) << "Push: " + _tuple.stream;
        }
        bool has_frame = false;
        if (checkPush) {
            // find nalu
            vector<string> nalus;
            std::string data = _lastPackage->body;
            int dataSize = data.length();
            int seek = 0;
            int start = -1;

            int lastType = 0;
            while (seek < dataSize) {
                int type = 0;
                if (start >= 0 && seek - start < 6) {
                    seek += 2;
                    continue;
                }
                int remaining = dataSize - seek;
                if (remaining >= 8 && data.substr(seek, 8) == "00000001") {
                    // start 4
                    type = 4;
                    lastType = 4;
                } else if (remaining >= 6 && data.substr(seek, 6) == "000001") {
                    // start 3
                    type = 3;
                    lastType = 3;
                }
                if (type != 0 || remaining <= 2) {
                    if (start >= 0) {
                        int lng = seek - start;
                        if (remaining <= 2) {
                            lng += 2;
                        }
                        std::string nalu = data.substr(start, lng);
                        nalus.push_back(nalu.substr(lastType * 2));
                    }
                    start = seek;
                }
                seek += 2;
            }
            const int MAX_PACKET = 1960;
            // build rtp package
            int toalNalu = nalus.size();
            vector<string> listRtp;
            std::string ssrc = _ssrcStr;
            std::string timeHex = "";
            std::stringstream timeSS;
            timeSS << std::setw(8) << std::setfill('0') << std::hex << _time;
            timeHex = timeSS.str();

            std::string headerRtp = "8062";
            for (size_t i = 0; i < toalNalu; i++) {
                std::string nalu = nalus[i];
                uint8_t naluHeader = strtol(nalu.substr(0, 2).c_str(), &p, 16);
                // | F (1 bit) | NRI (2 bit) | Type (5 bit) |
                uint8_t f = naluHeader >> 7 & 1; // 0 (Normal) | 1 (Error)
                uint8_t nrt = naluHeader >> 5 & 1; // Priority(0: low, 3: high)
                /*
                | **Type** | **Name** | **Description** |
                |----------|-----------------------------------|------------------------------------------------------------------------------|
                | 0 | Undefined | Not used. |
                | 1 | Coded Slice (Non-IDR) | Regular slice (P/B-frame). |
                | 5 | Coded Slice (IDR) | Initial slice GOP (I-frame), reset decoder. |
                | 6 | SEI (Supplemental Info) | Metadata (time, caption, watermark...). |
                | 7 | SPS (Sequence Parameter Set) | Sequence-level parameters (resolution, FPS, profile/level...). **Important**. |
                | 8 | PPS (Picture Parameter Set) | Picture-level parameters (entropy coding, slice groups...). **Important**. |
                | 9 | AUD (Access Unit Delimiter) | Marks the boundary between Access Units. |
                | 12 | Filler Data | Filler data to adjust bitrate. |
                */
                uint8_t type = naluHeader & 0x1F;
                /*
                ### **4. Example NALU Stream in GOP**
                A typical NALU sequence for an IDR frame:
                ```
                [SPS] → [PPS] → [SEI] → [IDR Slice] → [Non-IDR Slice] → ...
                ```
                - **SPS/PPS** appears at the beginning of the stream or when the parameter changes.
                - **IDR Slice** requires the decoder to re-initialize to avoid cumulative errors.
                */
                if (f == 1) {
                    // nalu error
                    continue;
                }
                int lengthNalu = nalu.size();
                has_frame = (type == 1 || type == 5);
                if (!(has_frame || type == 7 || type == 8)) {
                    continue;
                }
                // if (type == 1) {
                //     //_time = 90000;
                //     continue;
                // }

                if (lengthNalu <= MAX_PACKET) {
                    std::stringstream sqeSS;
                    sqeSS << std::setw(4) << std::setfill('0') << std::hex << _sqe;
                    std::string rtp = "";
                    rtp += headerRtp;
                    rtp += sqeSS.str(); // 0x0000 - sqe
                    rtp += timeHex; // 0x00000000 - time
                    rtp += ssrc; // 0x00000000 - ssrc
                    rtp += nalu;
                    listRtp.push_back(rtp);
                    _sqe += 1;
                } else {
                    int seekNalu = 0;
                    int indexBulk = 0;
                    uint8_t fu_indicator = (naluHeader & 0xE0) | 28; // FU-A type
                    uint8_t fu_header_start = type | 0x80;
                    uint8_t fu_header_end = type | 0x40;
                    std::stringstream fu_indicator_str;
                    fu_indicator_str << ((int)fu_indicator < 16 ? "0" : "") << std::hex << (int)fu_indicator;
                    std::stringstream fu_header_str;
                    fu_header_str << ((int)type < 16 ? "0" : "") << std::hex << (int)type;
                    std::stringstream fu_header_start_str;
                    fu_header_start_str << ((int)fu_header_start < 16 ? "0" : "") << std::hex << (int)fu_header_start;
                    std::stringstream fu_header_end_str;
                    fu_header_end_str << ((int)fu_header_end < 16 ? "0" : "") << std::hex << (int)fu_header_end;
                    while (seekNalu < lengthNalu) {
                        int naluRemain = lengthNalu - seekNalu;
                        std::stringstream sqeSS;
                        sqeSS << std::setw(4) << std::setfill('0') << std::hex << _sqe;
                        std::string rtp = "";
                        headerRtp = "8062";
                        rtp += sqeSS.str(); // 0x0000 - sqe
                        rtp += timeHex; // 0x00000000 - time
                        rtp += ssrc; // 0x00000000 - ssrc
                        rtp += fu_indicator_str.str();
                        int packetFlagNalu = 0x01;
                        if (indexBulk == 0) {
                            seekNalu += 2;
                            rtp += fu_header_start_str.str();
                        } else if (naluRemain <= MAX_PACKET) {
                            headerRtp = "80e2";
                            rtp += fu_header_end_str.str();
                        } else {
                            rtp += fu_header_str.str();
                        }
                        rtp = headerRtp + rtp;
                        if (naluRemain > MAX_PACKET) {
                            rtp += nalu.substr(seekNalu, MAX_PACKET);
                        } else {
                            rtp += nalu.substr(seekNalu, naluRemain);
                        }
                        seekNalu += MAX_PACKET;
                        _sqe += 1;
                        indexBulk += 1;
                        listRtp.push_back(rtp);
                    }
                }
            }

            // pusb rtp
            for (size_t i = 0; i < listRtp.size(); i++) {
                std::string rtpHex = listRtp[i];
                int rtpLength = rtpHex.length();
                vector<char> arrCharHex;
                //// check JT1078
                int rtpIndex = 0;
                while (rtpIndex < rtpLength) {
                    int val = stoi(rtpHex.substr(rtpIndex, 2), nullptr, 0x10);
                    arrCharHex.push_back((char)val);
                    rtpIndex += 2;
                }
                // writeFileName(rtpHex, "jt1078_rtp.txt");
                onRtpPacket(arrCharHex.data(), arrCharHex.size());
            }

            if (has_frame) {

                /*_time += (90000 / _fps);

                if (_lastPackage->frameInterval == 0) {
                    _lastFrameInterval = _lastPackage->frameInterval;
                }
                if (_lastPackage->frameInterval > _lastFrameInterval) {
                    _fps_temp += 1;
                } else {
                    if (_fps_temp > 0) {
                        _fps = _fps_temp;
                    }
                    _fps_temp = 0;
                }
                _lastFrameInterval = _lastPackage->frameInterval;*/

                auto now = chrono::system_clock::now();
                auto duration = now.time_since_epoch();
                uint64_t milliseconds = chrono::duration_cast<chrono::milliseconds>(duration).count();
                if (_firstTime == 0) {
                    _firstTime = milliseconds;
                }
                _time = (uint32_t)((milliseconds - _firstTime) * 90);
                /*    InfoP(this) << "iframeInterval type: _fps:" << _fps << " " << _lastPackage->dataType << " " << _lastPackage->iframeInterval << ", "
                                << _lastPackage->frameInterval;*/
                /*auto now = chrono::system_clock::now();
                auto duration = now.time_since_epoch();
                uint64_t milliseconds = chrono::duration_cast<chrono::milliseconds>(duration).count();
                if (_firstTime == 0) {
                    _firstTime = milliseconds;
                }
                _time = (uint32_t)((milliseconds - _firstTime) * 90);*/
            }
            _lastPackage->body = "";
        }

        return;
    }
}
void RtpSession::AnalysisJT1078264New(std::string strHex) {

    if (true) {
        std::stringstream ss;
        writeFileName(strHex, "jt1078_264.txt");
        JT1078Package *package = new JT1078Package();
        int index = 0;
        char *p;
        std::string logo = strHex.substr(index, 8);
        index += 8;
        package->header = strHex.substr(index, 4);
        index += 4;
        package->sqe = strHex.substr(index, 4);
        index += 4;
        package->imei = strHex.substr(index, 12);
        index += 12;
        package->channel = strHex.substr(index, 2);
        index += 2;
        std::string lab = strHex.substr(index, 2);
        int label3 = static_cast<int>(stoul(strHex.substr(index, 2), nullptr, 16) & 0xFF);
        int subPackageType = label3 & 0x0f;
        int dataType = label3 >> 4;
        uint64_t time = 0;
        index += 2;

        if (dataType != JT1078Package::DataType::TransparentData) {
            index += 8; // skip 4 byte time
            time = strtol(strHex.substr(index, 8).c_str(), &p, 16);
            index += 8;
            /* if (_firstTime == 0) {
                 _firstTime = time;
             }*/
            // package->timestamp = time;
        }

        if (dataType != JT1078Package::DataType::TransparentData && dataType != JT1078Package::DataType::Audio) {
            // std::string frameInterval = strHex.substr(index, 4);
            package->frameInterval = strtol(strHex.substr(index, 4).c_str(), nullptr, 16);
            index += 4;
            // std::string iframeInterval = strHex.substr(index, 4);
            package->iframeInterval = strtol(strHex.substr(index, 4).c_str(), nullptr, 16);
            index += 4;
        }
        /*if (dataType == JT1078Package::DataType::Audio) {
            return;
        }*/
        package->length = strtol(strHex.substr(index, 4).c_str(), &p, 16);
        index += 4;
        package->body = strHex.substr(index);
        bool checkPush = false;
        switch (subPackageType) {
            case JT1078Package::SubPackageType::FirstPacket: _lastPackage = package; break;
            case JT1078Package::SubPackageType::SubPacket: _lastPackage->body += package->body; break;
            case JT1078Package::SubPackageType::LastPacket:
                _lastPackage->body += package->body;
                checkPush = true;
                break;
            case JT1078Package::SubPackageType::Fixed:
                _lastPackage = package;
                checkPush = true;
                break;
            default: break;
        }
        if (checkPush && _sqe == 0) {
            InfoP(this) << "Push: " + _tuple.stream;
        }
        if (checkPush) {
            // find nalu
            vector<string> nalus;
            std::string data = _lastPackage->body;
            int dataSize = data.length();
            int seek = 0;
            int start = -1;

            int lastType = 0;
            while (seek < dataSize) {
                int type = 0;
                if (start >= 0 && seek - start < 6) {
                    seek += 2;
                    continue;
                }
                int remaining = dataSize - seek;
                if (remaining >= 8 && data.substr(seek, 8) == "00000001") {
                    // start 4
                    type = 4;
                    lastType = 4;
                } else if (remaining >= 6 && data.substr(seek, 6) == "000001") {
                    // start 3
                    type = 3;
                    lastType = 3;
                }
                if (type != 0 || remaining <= 2) {
                    if (start >= 0) {
                        int lng = seek - start;
                        if (remaining <= 2) {
                            lng += 2;
                        }
                        std::string nalu = data.substr(start, lng);
                        nalus.push_back(nalu.substr(lastType * 2));
                    }
                    start = seek;
                }
                seek += 2;
            }
            const int MAX_PACKET = 1960;
            // build rtp package
            int toalNalu = nalus.size();
            vector<string> listRtp;
            std::string ssrc = _ssrcStr;
            std::string timeHex = "";
            std::stringstream timeSS;
            timeSS << std::setw(8) << std::setfill('0') << std::hex << _time;
            timeHex = timeSS.str();

            // std::string headerRtp = "8062";
            std::string headerRtp = "8062";
            bool checkIframe = false;
            for (size_t i = 0; i < toalNalu; i++) {
                std::string nalu = nalus[i];
                // build rtp
                int naluType = strtol(nalu.substr(0, 2).c_str(), &p, 16);
                int lengthNalu = nalu.size();
                int naluTypeCheck = naluType & 0x1f;
                if (naluType == 0x65) {
                    checkIframe = true;
                }

                if (lengthNalu <= MAX_PACKET) {
                    std::stringstream sqeSS;
                    sqeSS << std::setw(4) << std::setfill('0') << std::hex << _sqe;
                    std::string rtp = "";
                    rtp += headerRtp;
                    rtp += sqeSS.str(); // 0x0000 - sqe
                    rtp += timeHex; // 0x00000000 - time
                    rtp += ssrc; // 0x00000000 - ssrc
                    rtp += nalu;
                    listRtp.push_back(rtp);
                    _sqe += 1;
                } else {
                    int seekNalu = 0;
                    int indexBulk = 0;
                    while (seekNalu < lengthNalu) {
                        int naluRemain = lengthNalu - seekNalu;
                        std::stringstream sqeSS;
                        sqeSS << std::setw(4) << std::setfill('0') << std::hex << _sqe;
                        std::string rtp = "";
                        headerRtp = "8062";

                        rtp += sqeSS.str(); // 0x0000 - sqe
                        rtp += timeHex; // 0x00000000 - time
                        rtp += ssrc; // 0x00000000 - ssrc
                        int packetTypeFu = (naluType & 0x60) | 28;
                        std::stringstream packetTypeFuss;
                        packetTypeFuss << (packetTypeFu < 16 ? "0" : "") << std::hex << packetTypeFu;
                        rtp += packetTypeFuss.str();
                        int packetFlagNalu = 0x01;
                        if (indexBulk == 0) {
                            // first
                            packetFlagNalu = (packetTypeFu & 0x1f) | 0x80;
                            if (packetTypeFu == 124) {
                                packetFlagNalu = 133;
                            } else if (packetTypeFu == 92) {
                                packetFlagNalu = 129;
                            }
                            seekNalu += 2;

                        } else if (naluRemain <= MAX_PACKET) {
                            // end
                            packetFlagNalu = (packetTypeFu & 0x1f) | 0x40;
                            if (packetTypeFu == 124) {
                                packetFlagNalu = 69;
                            } else if (packetTypeFu == 92) {
                                packetFlagNalu = 65;
                            }
                            headerRtp = "80e2";
                        }
                        rtp = headerRtp + rtp;
                        std::stringstream packetFlagNaluSS;
                        packetFlagNaluSS << (packetFlagNalu < 16 ? "0" : "") << std::hex << packetFlagNalu;
                        rtp += packetFlagNaluSS.str();
                        if (naluRemain > MAX_PACKET) {
                            rtp += nalu.substr(seekNalu, MAX_PACKET);
                        } else {
                            rtp += nalu.substr(seekNalu, naluRemain);
                        }
                        seekNalu += MAX_PACKET;
                        _sqe += 1;
                        indexBulk += 1;
                        listRtp.push_back(rtp);
                    }
                    checkIframe = true;
                }
            }

            // pusb rtp
            for (size_t i = 0; i < listRtp.size(); i++) {
                std::string rtpHex = listRtp[i];
                int rtpLength = rtpHex.length();
                vector<char> arrCharHex;
                //// check JT1078
                int rtpIndex = 0;
                while (rtpIndex < rtpLength) {
                    int val = stoi(rtpHex.substr(rtpIndex, 2), nullptr, 0x10);
                    arrCharHex.push_back((char)val);
                    rtpIndex += 2;
                }
                writeFileName(rtpHex, "jt1078_rtp.txt");
                onRtpPacket(arrCharHex.data(), arrCharHex.size());
            }
            /*if (package->iframeInterval > 0) {
                int delta = 0;
                delta = (double)(package->iframeInterval / 10);
                _time += (int)(delta > 0 ? 90000 / delta : 90000 / 10);
            } else {
                _time += (int)(90000 / 10);
            }*/
            /*_time += (int)(90000 / 15);*/
            /*    if (checkIframe) {
                    _time += (int)(90000 / 15);
                }*/
            /*auto now = chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            uint64_t milliseconds = chrono::duration_cast<chrono::milliseconds>(duration).count();
            if (_firstTime == 0) {
                _firstTime = milliseconds;
            }
            _time = (uint32_t)((milliseconds - _firstTime)* 80);       */
            _time += 90000 / 15;
            _lastPackage->body = "";
        }

        return;
    }
}

#pragma endregion

bool byteToHex(char *buff, int length) {

    std::stringstream ss;
    for (size_t i = 0; i < length; i++) {
        int val = buff[i];
        ss << (val < 16 ? "0" : "") << std::hex << val;
    }
    std::string demo = ss.str();
    return true;
}
// uint32_t bigEndianToNative(const uint8_t *data) {
//
//    uint32_t value;
//    std::memcpy(&value, data, sizeof(value));
//    uint8_t *bytes = reinterpret_cast<uint8_t *>(&value);
//    std::reverse(bytes, bytes + sizeof(value));
//
//    return value;
//}

// uint64_t ReadByte(char* buff, int index, int length) {
//
// }

bool isJT1078(const char *buf, size_t size) {
    if (size < 4) {
        return false;
    }
    bool check = (uint8_t)buf[0] == 0x30 && (uint8_t)buf[1] == 0x31 && (uint8_t)buf[2] == 0x63 && (uint8_t)buf[3] == 0x64;
    return check;
}
// 303163648162000000190130503701010000017671C54EBD0000000003B600000001674D001495A85825900000000168EE3C800000000106E5017F800000000165B800000CDFE0BFEA0867DCFF03C93CFDAA40567E6C9F4B556EF829ECEFDBFEC88DDD904E22DA0A3B012C622EA56C6C051301BE59457165786A2307E87365A4CDD9E64BCA86B4CC2BF9376CE3C072490FA6FDFFB60C66B5274E2212359F16084B8E5C1CEAFD68A1F0A381366F74B807BD854E05708C04CF69471CA2FCC96EC55511E0448AAE1773E182DBF82B7996F3297AAFB7426008D204ED4D47076238095298A4C5C565D388816C8E28A9C23C2941166B035EEE95071184039790061E3241C6BCDCB447E63FBA3A57118498556663A1C445E14A7DBF098345EB5D3FF90B677799E6E85827438EC0F9B4479E25F8D2CC0BD126ABF36D1BF79AA65F8D2E7AC0ABF65E4E76C3B5ACCEE2350F3FA4F8D73344F7691B1C5613417C6C151C43F9F0654AADC7B6122D4B68AA0ECC62A10FE598348DE24C56B38A7F5EFCEB908AF8A44C5C896FE7FA93E71DAA259B3483468FCA6EDB86AE19C838739B7D50B19C462214CB0BF54CE70452FB3830FB8F3D89A99CB24D0867A8957FC1014289215B46D5B96ADF9C53263A3E77EBAE1588B210B79A29342D54444F9F251E360C479F4E758F3D64BCBB8FFFB20A934836AE59ED0C431852FF82EA0E614E23BE4F6BEDA2490A5658A44461BFFF9BF516CFDF7298BE5D8AFA0A14804308CCB97BD023332DD59F25F822F1A246D40BE18EE1B09BA07AC55C959461B07D244D91AF5969A1AEEA8B61EA52B3D4F627E7DDA4B7B533E74F3D309764F997DC16B99FA960EDD91157A71DADB1D03F0ECEC5B0A75C93173D2827AED4F0971C9E60270BDE63BF2A675EE9278580D9A765A195C4F3E0090E01339253E89182BC780D0EA00C2311E99D31130EB73941F151B50D97B01FF620D2E23F194591B7FF167DE1F008965A7B131FBE32DE1577FF6F4FC59DA969FDFBB9D0CBA3103E5D3B59407BFB5076762046844E86A8F9A4550D1A111708E03211B29AF06A4839887D614EF658CA2464977503F9C2F253A063735BFF3BD8114B879A7469140C6F1196DDD5425998F8D804D9925D48B36F5E328A9041FB3B86DD2E3ADDAC53621DBCD049DBBFFB7A04813E44E3F043B3D542FAA0594987805459A7CB93DC3E3D5C804AD6D7CD913B1E753197C8BD5ED5FCCDD06F4725284D044AEBC93ACF69BD1714FE10C1A71ABE1B8F8DDC72E9DE150A4525060E4F7D527156D394DE1733630349A5BB64F0B884CA4AF85B92F8463F341FD0CCB7A1061CF3A7E823702267CF4461C54D7B0B742E2CBBA153C230789A934F4D8A2F3B44BEFC53FC91627CC838
void RtpSession::onRecv(const Buffer::Ptr &data) {
    // isJT1078(data->data(), data->size())
    std::stringstream ss;
    if (true) {

        int length = data->size();
        char *buffData = data->data();
        int label3 = 0;
        for (size_t i = 0; i < length; i++) {
            int val = (uint8_t)buffData[i];
            ss << (val < 16 ? "0" : "") << std::hex << val;
            if (i == 4 + 4 + 6 + 1) {
                label3 = val;
            }
        }
        // std::string strHex = _reverse + ss.str();
        // writeFileName(ss.str(), "rtp_h265.txt");
    }
    if (isJT1078(data->data(), data->size()) || !_imei.empty()) {
        /*    _tuple.app = "live";
            _tuple.stream = "01";*/
        _is_udp = true;
        std::stringstream ss;
        int length = data->size();
        char *buffData = data->data();
        int label3 = 0;
        for (size_t i = 0; i < length; i++) {
            int val = (uint8_t)buffData[i];
            ss << (val < 16 ? "0" : "") << std::hex << val;
            if (i == 4 + 4 + 6 + 1) {
                label3 = val;
            }
        }
        std::string strHex = _reverse + ss.str();
        int lengthHex = strHex.size();
        int seek = 0;
        int start = -1;
        // 000000322433
        //   filter JT1078
        vector<string> listJT1078;
        while (seek < lengthHex) {
            int remain = lengthHex - seek;
            bool checkFind = false;
            if (remain > 50) {
                if (strHex.substr(seek, 8) == "30316364" && (strHex.substr(seek + 16, 12) == _imei || _imei.empty())) {
                    checkFind = true;
                    if (_imei.empty()) {
                        _imei = strHex.substr(seek + 16, 12);
                        _channel = strHex.substr(seek + 29, 1);
                        _tuple.stream = _channel + "/" + _imei;
                        _tuple.app = "live";
                        const int STR_LEN = 8;
                        std::string ssrcStr = "";
                        const char *hex_digits = "0123456789ABCDEF";
                        int i;

                        for (i = 0; i < STR_LEN; i++) {
                            ssrcStr += hex_digits[(rand() % 16)];
                        }

                        _ssrcStr = ssrcStr;
                        InfoP(this) << "Start: " + _tuple.stream;
                    }
                    /* int dataType = strtol(strHex.substr(30, 2).c_str(), nullptr, 16) >> 4;
                     int lengthIndex = 32;
                     if (dataType != JT1078Package::DataType::TransparentData) {
                         lengthIndex += 16;
                     }
                     if (dataType != JT1078Package::DataType::TransparentData && dataType != JT1078Package::DataType::Audio) {
                         lengthIndex += 8;
                     }
                     int lengthPackage = strtol(strHex.substr(seek + lengthIndex, 4).c_str(), nullptr, 16) * 2;
                     if (lengthPackage > 980 * 2) {
                         lengthPackage = 980 * 2;
                     }
                     if (remain >= lengthPackage) {
                         int fullLength = lengthIndex + lengthPackage + 4;
                         std::string package = strHex.substr(seek, fullLength);
                         listJT1078.push_back(package);
                         seek += fullLength;
                         start = -1;
                         continue;
                     }     */
                }
            }
            if (checkFind) {
                if (start >= 0) {
                    std::string package = strHex.substr(start, seek - start);
                    listJT1078.push_back(package);
                }
                start = seek;
                _reverse = strHex.substr(seek);
            }
            /* if (start > 0 && remain < 15) {

             }
             if (start >= 0 && remain <= 50) {
                 std::string package = strHex.substr(start);
                 listJT1078.push_back(package);
                 break;
             }*/
            seek += 2;
        }
        int lengthPackage = listJT1078.size();
        for (size_t i = 0; i < lengthPackage; i++) {
            AnalysisJT1078264(listJT1078[i]);
        }
        return;
    }
    if (_is_udp) {
        onRtpPacket(data->data(), data->size());
        return;
    }

    // InfoP(this) << "Data rtp packet";

    RtpSplitter::input(data->data(), data->size());
}
void onRecv31231(const Buffer::Ptr &data) {
    if (isJT1078(data->data(), data->size())) {
        /*    _tuple.app = "live";
            _tuple.stream = "01";*/
        //_is_udp = true;
        // std::stringstream ss;
        // int length = data->size();
        // char *buffData = data->data();
        // int label3 = 0;
        // for (size_t i = 0; i < length; i++) {
        //    int val = (uint8_t)buffData[i];
        //    ss << (val < 16 ? "0" : "") << std::hex << val;
        //    if (i == 4 + 4 + 6 + 1) {
        //        label3 = val;
        //    }
        //}
        // std::string strHex = ss.str();
        // int lengthHex = strHex.size();
        // int seek = 0;
        // int start = -1;
        //// 000000322433
        ////   filter JT1078
        // vector<string> listJT1078;
        // while (seek < lengthHex) {
        //     int remain = lengthHex - seek;
        //     bool checkFind = false;
        //     if (remain > 50) {
        //         if (strHex.substr(seek, 8) == "30316364" && strHex.substr(seek + 16, 12) == "000000322433") {
        //             checkFind = true;
        //         }
        //     }
        //     if (checkFind) {
        //         if (start >= 0) {
        //             std::string package = strHex.substr(start, seek - start);
        //             listJT1078.push_back(package);
        //         }
        //         start = seek;
        //     }
        //     if (start >= 0 && remain <= 50) {
        //         std::string package = strHex.substr(start);
        //         listJT1078.push_back(package);
        //         break;
        //     }
        //     seek += 2;
        // }
        // int lengthPackage = listJT1078.size();
        // for (size_t i = 0; i < lengthPackage; i++) {
        //     AnalysisJT1078(listJT1078[i]);
        // }
        /*  string hex =
          "303163648162000000190130503701010000017671C54EBD0000000003B600000001674D001495A85825900000000168EE3C800000000106E5017F800000000165B800000CDFE"
                       "0BFEA0867DCFF03C93CFDAA40567E6C9F4B556EF829ECEFDBFEC88DDD904E22DA0A3B012C622EA56C6C051301BE59457165786A2307E87365A4CDD9E64BCA86B4CC2BF9376CE3"
                       "C072490FA6FDFFB60C66B5274E2212359F16084B8E5C1CEAFD68A1F0A381366F74B807BD854E05708C04CF69471CA2FCC96EC55511E0448AAE1773E182DBF82B7996F3297AAFB"
                       "7426008D204ED4D47076238095298A4C5C565D388816C8E28A9C23C2941166B035EEE95071184039790061E3241C6BCDCB447E63FBA3A57118498556663A1C445E14A7DBF0983"
                       "45EB5D3FF90B677799E6E85827438EC0F9B4479E25F8D2CC0BD126ABF36D1BF79AA65F8D2E7AC0ABF65E4E76C3B5ACCEE2350F3FA4F8D73344F7691B1C5613417C6C151C43F9F"
                       "0654AADC7B6122D4B68AA0ECC62A10FE598348DE24C56B38A7F5EFCEB908AF8A44C5C896FE7FA93E71DAA259B3483468FCA6EDB86AE19C838739B7D50B19C462214CB0BF54CE7"
                       "0452FB3830FB8F3D89A99CB24D0867A8957FC1014289215B46D5B96ADF9C53263A3E77EBAE1588B210B79A29342D54444F9F251E360C479F4E758F3D64BCBB8FFFB20A934836A"
                       "E59ED0C431852FF82EA0E614E23BE4F6BEDA2490A5658A44461BFFF9BF516CFDF7298BE5D8AFA0A14804308CCB97BD023332DD59F25F822F1A246D40BE18EE1B09BA07AC55C95"
                       "9461B07D244D91AF5969A1AEEA8B61EA52B3D4F627E7DDA4B7B533E74F3D309764F997DC16B99FA960EDD91157A71DADB1D03F0ECEC5B0A75C93173D2827AED4F0971C9E60270"
                       "BDE63BF2A675EE9278580D9A765A195C4F3E0090E01339253E89182BC780D0EA00C2311E99D31130EB73941F151B50D97B01FF620D2E23F194591B7FF167DE1F008965A7B131F"
                       "BE32DE1577FF6F4FC59DA969FDFBB9D0CBA3103E5D3B59407BFB5076762046844E86A8F9A4550D1A111708E03211B29AF06A4839887D614EF658CA2464977503F9C2F253A0637"
                       "35BFF3BD8114B879A7469140C6F1196DDD5425998F8D804D9925D48B36F5E328A9041FB3B86DD2E3ADDAC53621DBCD049DBBFFB7A04813E44E3F043B3D542FAA0594987805459"
                       "A7CB93DC3E3D5C804AD6D7CD913B1E753197C8BD5ED5FCCDD06F4725284D044AEBC93ACF69BD1714FE10C1A71ABE1B8F8DDC72E9DE150A4525060E4F7D527156D394DE1733630"
                       "349A5BB64F0B884CA4AF85B92F8463F341FD0CCB7A1061CF3A7E823702267CF4461C54D7B0B742E2CBBA153C230789A934F4D8A2F3B44BEFC53FC91627CC838";
         */
        // vector<char> arrCharHex;
        // vector<int> arrIntHex;
        //// check JT1078

        //// 30316364
        // for (size_t i = 8; i < hex.length(); i= i + 2) {
        //     int val = stoi(hex.substr(i, 2), nullptr, 0x10);
        //     arrIntHex.push_back(val);
        //     arrCharHex.push_back((char)val);
        // }
        // onRtpPacket(arrCharHex.data(), arrCharHex.size());
        // bool isJT1078 = false;
        // std::stringstream ss;
        // if (_is_udp) {
        //    vector<char> arr;
        //    char *buf = data->data();
        //    int size = data->size();
        //    vector<int> arrVal;
        //    //// check jt1078
        //    size_t i = 0;
        //    //// 30316364
        //    // if (size > 10) {
        //    //     isJT1078 = (uint8_t)buf[0] == 0x30
        //    //         && (uint8_t)buf[1] == 0x31
        //    //         && (uint8_t)buf[2] == 0x63
        //    //         && (uint8_t)buf[3] == 0x64;
        //    //     if (isJT1078) {
        //    //         i = 4;
        //    //     }
        //    // }
        //    for (i = 0; i < size; i++) {
        //        int val = (uint8_t)buf[i];
        //        arrVal.push_back(val);
        //        arr.push_back(buf[i]);
        //        ss << (val < 16 ? "0" : "") << std::hex << val;
        //    }
        //    std::string mystr = ss.str();
        //    char *dataProcessed = arr.data();
        //    // cout << "data: " << mystr;
        //    writeFile(mystr + "\n");
        //    onRtpPacket(data->data(), data->size());
        //    return;
        //}

        // RtpSplitter::input(data->data(), data->size());
    }
}
void onRecvDemo(const Buffer::Ptr &data) {
    // string hex =
    // "303163648162000000190130503701010000017671C54EBD0000000003B600000001674D001495A85825900000000168EE3C800000000106E5017F800000000165B800000CDFE"
    //              "0BFEA0867DCFF03C93CFDAA40567E6C9F4B556EF829ECEFDBFEC88DDD904E22DA0A3B012C622EA56C6C051301BE59457165786A2307E87365A4CDD9E64BCA86B4CC2BF9376CE3"
    //              "C072490FA6FDFFB60C66B5274E2212359F16084B8E5C1CEAFD68A1F0A381366F74B807BD854E05708C04CF69471CA2FCC96EC55511E0448AAE1773E182DBF82B7996F3297AAFB"
    //              "7426008D204ED4D47076238095298A4C5C565D388816C8E28A9C23C2941166B035EEE95071184039790061E3241C6BCDCB447E63FBA3A57118498556663A1C445E14A7DBF0983"
    //              "45EB5D3FF90B677799E6E85827438EC0F9B4479E25F8D2CC0BD126ABF36D1BF79AA65F8D2E7AC0ABF65E4E76C3B5ACCEE2350F3FA4F8D73344F7691B1C5613417C6C151C43F9F"
    //              "0654AADC7B6122D4B68AA0ECC62A10FE598348DE24C56B38A7F5EFCEB908AF8A44C5C896FE7FA93E71DAA259B3483468FCA6EDB86AE19C838739B7D50B19C462214CB0BF54CE7"
    //              "0452FB3830FB8F3D89A99CB24D0867A8957FC1014289215B46D5B96ADF9C53263A3E77EBAE1588B210B79A29342D54444F9F251E360C479F4E758F3D64BCBB8FFFB20A934836A"
    //              "E59ED0C431852FF82EA0E614E23BE4F6BEDA2490A5658A44461BFFF9BF516CFDF7298BE5D8AFA0A14804308CCB97BD023332DD59F25F822F1A246D40BE18EE1B09BA07AC55C95"
    //              "9461B07D244D91AF5969A1AEEA8B61EA52B3D4F627E7DDA4B7B533E74F3D309764F997DC16B99FA960EDD91157A71DADB1D03F0ECEC5B0A75C93173D2827AED4F0971C9E60270"
    //              "BDE63BF2A675EE9278580D9A765A195C4F3E0090E01339253E89182BC780D0EA00C2311E99D31130EB73941F151B50D97B01FF620D2E23F194591B7FF167DE1F008965A7B131F"
    //              "BE32DE1577FF6F4FC59DA969FDFBB9D0CBA3103E5D3B59407BFB5076762046844E86A8F9A4550D1A111708E03211B29AF06A4839887D614EF658CA2464977503F9C2F253A0637"
    //              "35BFF3BD8114B879A7469140C6F1196DDD5425998F8D804D9925D48B36F5E328A9041FB3B86DD2E3ADDAC53621DBCD049DBBFFB7A04813E44E3F043B3D542FAA0594987805459"
    //              "A7CB93DC3E3D5C804AD6D7CD913B1E753197C8BD5ED5FCCDD06F4725284D044AEBC93ACF69BD1714FE10C1A71ABE1B8F8DDC72E9DE150A4525060E4F7D527156D394DE1733630"
    //              "349A5BB64F0B884CA4AF85B92F8463F341FD0CCB7A1061CF3A7E823702267CF4461C54D7B0B742E2CBBA153C230789A934F4D8A2F3B44BEFC53FC91627CC838";
    // vector<char> arrCharHex;
    // vector<int> arrIntHex;
    //// 30316364
    // for (size_t i = 8; i < hex.length(); i = i + 2) {
    //     int val = stoi(hex.substr(i, 2), nullptr, 0x10);
    //     arrIntHex.push_back(val);
    //     arrCharHex.push_back((char)val);
    // }
    // onRtpPacket(arrCharHex.data(), arrCharHex.size());

    // std::stringstream ss;
    // if (_is_udp || true) {
    //     vector<char> arr;
    //     char *buf = data->data();
    //     int size = data->size();
    //     vector<int> arrVal;
    //     for (size_t i = 0; i < size; i++) {
    //         int val = (uint8_t)buf[i];
    //         arrVal.push_back(val);
    //         arr.push_back(buf[i]);
    //         ss << std::hex << val;
    //     }
    //     std::string mystr = ss.str();
    //     char *dataProcessed = arr.data();
    //     onRtpPacket(dataProcessed, arr.size());
    //     return;
    // }

    // RtpSplitter::input(data->data(), data->size());
}
// void RtpSession::onRecv(const Buffer::Ptr &data) {
//     //std::ofstream out("somefile.bin", std::ios::binary);
//     //FILE *file = fopen("demo.bin", "wb");
//     //fwrite(data->data(), 1, 100, file);
//     //string food = "Pizza"; // A food variable of type string
//     //string *ptr = &food; // A pointer variable, with the name ptr, that stores the address of food
//     Buffer::Ptr newData = data;
//     //int *size = data->size();
//     char *_data = data->data();
//     int length = strlen(_data);
//     //cout << "Hello World!";
//     //InfoP(this) << "Type name: " << typeid(_data).name();
//     std::stringstream ss;
//     const int size = newData->size();
//
//     //vector<int> arr;
//     vector<char> arrChar;
//     //char *dataProcess = new char[newData->size()];
//     for (int i = 0; i < newData->size(); ++i)
//     {
//         /*if (i > 10) {
//             arrChar.push_back(_data[i]);
//         }  */
//         arrChar.push_back(_data[i]);
//         //dataProcess[i] = _data[i];
//         int b = (uint8_t)_data[i];
//         //b = b > 0 ? b : 256 - b;
//         //arr.push_back(b);
//         ss << std::hex << b;
//     }
//
//     std::string mystr = ss.str();
//     //InfoP(this) << "hex: " << mystr;
//
//
//     //char* dataProcess = std:istringstream hex_chars_stream(mystr);
//     writeFile(mystr + "\n");
//     if (_is_udp) {
//         onRtpPacket(arrChar.data(), data->size());
//         return;
//     }
//
//     //InfoP(this) << "Data rtp packet";
//
//     RtpSplitter::input(data->data(), data->size());
// }

char *onProcessData(char *data, int size) {
    vector<char> arr;
    for (size_t i = 0; i < size; i++) {
        arr.push_back(data[i]);
    }
    return arr.data();
}

void RtpSession::onError(const SockException &err) {
    if (_emit_detach) {
        _process->onDetach(err);
    }
    WarnP(this) << _tuple.shortUrl() << " " << err;
}

void RtpSession::onManager() {
    if (!_process && _ticker.createdTime() > 10 * 1000) {
        shutdown(SockException(Err_timeout, "illegal connection"));
    }
}

void RtpSession::setRtpProcess(RtpProcess::Ptr process) {
    _emit_detach = (bool)process;
    _process = std::move(process);
}

void RtpSession::onRtpPacket(const char *data, size_t len) {
    if (!isRtp(data, len)) {
        // 忽略非rtp数据  [AUTO-TRANSLATED:771b77d8]
        // Ignore non-rtp data
        WarnP(this) << "Not rtp packet";
        return;
    }
    if (!_is_udp) {
        if (_search_rtp) {
            // 搜索上下文期间，数据丢弃  [AUTO-TRANSLATED:e0a3b407]
            // Data discarded during context search
            if (_search_rtp_finished) {
                // 下个包开始就是正确的rtp包了  [AUTO-TRANSLATED:a73a3a61]
                // The next packet is the correct rtp packet
                _search_rtp_finished = false;
                _search_rtp = false;
            }
            return;
        }
        GET_CONFIG(uint32_t, rtpMaxSize, Rtp::kRtpMaxSize);
        if (len > 1024 * rtpMaxSize) {
            _search_rtp = true;
            WarnL << "rtp包长度异常(" << len << ")，发送端可能缓存溢出并覆盖，开始搜索ssrc以便恢复上下文";
            return;
        }
    }

    // 未设置ssrc时，尝试获取ssrc  [AUTO-TRANSLATED:30f31a81]
    // Try to get ssrc when ssrc is not set
    if (!_ssrc && !getSSRC(data, len, _ssrc)) {
        return;
    }

    // 未指定流id就使用ssrc为流id  [AUTO-TRANSLATED:9eb98394]
    // Use ssrc as stream id if stream id is not specified
    if (_tuple.stream.empty()) {
        _tuple.stream = printSSRC(_ssrc);
    }

    if (!_process) {
        _process = RtpProcess::createProcess(_tuple);
        _process->setOnlyTrack((RtpProcess::OnlyTrack)_only_track);
        weak_ptr<RtpSession> weak_self = static_pointer_cast<RtpSession>(shared_from_this());
        _process->setOnDetach([weak_self](const SockException &ex) {
            if (auto strong_self = weak_self.lock()) {
                strong_self->safeShutdown(ex);
            }
        });
    }
    try {
        uint32_t rtp_ssrc = 0;
        getSSRC(data, len, rtp_ssrc);
        if (rtp_ssrc != _ssrc) {
            WarnP(this) << "ssrc mismatched, rtp dropped: " << rtp_ssrc << " != " << _ssrc;
            return;
        }
        _process->inputRtp(false, getSock(), data, len, (struct sockaddr *)&_addr);
    } catch (RtpTrack::BadRtpException &ex) {
        if (!_is_udp) {
            WarnL << ex.what() << "，开始搜索ssrc以便恢复上下文";
            _search_rtp = true;
        } else {
            throw;
        }
    }
    _ticker.resetTime();
}

static const char *findSSRC(const char *data, ssize_t len, uint32_t ssrc) {
    // rtp前面必须预留两个字节的长度字段  [AUTO-TRANSLATED:2af4e647]
    // Two bytes of length field must be reserved before rtp
    for (ssize_t i = 2; i <= len - 4; ++i) {
        auto ptr = (const uint8_t *)data + i;
        if (ptr[0] == (ssrc >> 24) && ptr[1] == ((ssrc >> 16) & 0xFF) && ptr[2] == ((ssrc >> 8) & 0xFF) && ptr[3] == (ssrc & 0xFF)) {
            return (const char *)ptr;
        }
    }
    return nullptr;
}

static const char *findPsHeaderFlag(const char *data, ssize_t len) {
    for (ssize_t i = 2; i <= len - 4; ++i) {
        auto ptr = (const uint8_t *)data + i;
        // PsHeader 0x000001ba、PsSystemHeader0x000001bb（关键帧标识）  [AUTO-TRANSLATED:f8146534]
        // PsHeader 0x000001ba, PsSystemHeader 0x000001bb (keyframe identifier)
        if (ptr[0] == (0x00) && ptr[1] == (0x00) && ptr[2] == (0x01) && ptr[3] == (0xbb)) {
            return (const char *)ptr;
        }
    }

    return nullptr;
}

// rtp长度到ssrc间的长度固定为10  [AUTO-TRANSLATED:7428bd59]
// The length between rtp length and ssrc is fixed to 10
static size_t constexpr kSSRCOffset = 2 + 4 + 4;
// rtp长度到ps header间的长度固定为14 （暂时不采用找ps header,采用找system header代替）  [AUTO-TRANSLATED:cf6b289c]
// The length between rtp length and ps header is fixed to 14 (temporarily not using ps header, using system header instead)
// rtp长度到ps system header间的长度固定为20 (关键帧标识)  [AUTO-TRANSLATED:abe8bb8e]
// The length between rtp length and ps system header is fixed to 20 (keyframe identifier)
static size_t constexpr kPSHeaderOffset = 2 + 4 + 4 + 4 + 20;

const char *RtpSession::onSearchPacketTail(const char *data, size_t len) {
    if (!_search_rtp) {
        // tcp上下文正常，不用搜索ssrc  [AUTO-TRANSLATED:cab86669]
        // Tcp context is normal, no need to search ssrc
        return RtpSplitter::onSearchPacketTail(data, len);
    }
    if (!_process) {
        InfoL << "ssrc未获取到，无法通过ssrc恢复tcp上下文；尝试搜索PsSystemHeader恢复tcp上下文。";
        auto rtp_ptr1 = searchByPsHeaderFlag(data, len);
        return rtp_ptr1;
    }
    auto rtp_ptr0 = searchBySSRC(data, len);
    if (rtp_ptr0) {
        return rtp_ptr0;
    }
    // ssrc搜索失败继续尝试搜索ps header flag  [AUTO-TRANSLATED:e8f65bd2]
    // Continue to search for ps header flag if ssrc search fails
    auto rtp_ptr2 = searchByPsHeaderFlag(data, len);
    return rtp_ptr2;
}

const char *RtpSession::searchBySSRC(const char *data, size_t len) {
    InfoL << "尝试rtp搜索ssrc..._ssrc=" << _ssrc;
    // 搜索第一个rtp的ssrc  [AUTO-TRANSLATED:6b010df0]
    // Search for the first rtp's ssrc
    auto ssrc_ptr0 = findSSRC(data, len, _ssrc);
    if (!ssrc_ptr0) {
        // 未搜索到任意rtp，返回数据不够  [AUTO-TRANSLATED:50db17ed]
        // Return insufficient data if no rtp is found
        InfoL << "rtp搜索ssrc失败（第一个数据不够），丢弃rtp数据为：" << len;
        return nullptr;
    }
    // 这两个字节是第一个rtp的长度字段  [AUTO-TRANSLATED:75816ba4]
    // These two bytes are the length field of the first rtp
    auto rtp_len_ptr = (ssrc_ptr0 - kSSRCOffset);
    auto rtp_len = ((uint8_t *)rtp_len_ptr)[0] << 8 | ((uint8_t *)rtp_len_ptr)[1];

    // 搜索第二个rtp的ssrc  [AUTO-TRANSLATED:238eaa43]
    // Search for the second rtp's ssrc
    auto ssrc_ptr1 = findSSRC(ssrc_ptr0 + rtp_len, data + (ssize_t)len - ssrc_ptr0 - rtp_len, _ssrc);
    if (!ssrc_ptr1) {
        // 未搜索到第二个rtp，返回数据不够  [AUTO-TRANSLATED:3a78a586]
        // Return insufficient data if the second rtp is not found
        InfoL << "rtp搜索ssrc失败(第二个数据不够)，丢弃rtp数据为：" << len;
        return nullptr;
    }

    // 两个ssrc的间隔正好等于rtp的长度(外加rtp长度字段)，那么说明找到rtp  [AUTO-TRANSLATED:b1517bfd]
    // The interval between the two ssrcs is exactly equal to the length of the rtp (plus the rtp length field), which means that the rtp is found
    auto ssrc_offset = ssrc_ptr1 - ssrc_ptr0;
    if (ssrc_offset == rtp_len + 2 || ssrc_offset == rtp_len + 4) {
        InfoL << "rtp搜索ssrc成功，tcp上下文恢复成功，丢弃的rtp残余数据为：" << rtp_len_ptr - data;
        _search_rtp_finished = true;
        if (rtp_len_ptr == data) {
            // 停止搜索rtp，否则会进入死循环  [AUTO-TRANSLATED:319eefa7]
            // Stop searching for rtp, otherwise it will enter an infinite loop
            _search_rtp = false;
        }
        // 前面的数据都需要丢弃，这个是rtp的起始  [AUTO-TRANSLATED:129082d2]
        // All previous data needs to be discarded, this is the start of rtp
        return rtp_len_ptr;
    }
    // 第一个rtp长度不匹配，说明第一个找到的ssrc不是rtp，丢弃之，我们从第二个ssrc所在rtp开始搜索  [AUTO-TRANSLATED:ec35b2ba]
    // The length of the first rtp does not match, which means that the first ssrc found is not rtp, discard it, we start searching from the second ssrc
    // rtp
    return ssrc_ptr1 - kSSRCOffset;
}

const char *RtpSession::searchByPsHeaderFlag(const char *data, size_t len) {
    InfoL << "尝试rtp搜索PsSystemHeaderFlag..._ssrc=" << _ssrc;
    // 搜索rtp中的第一个PsHeaderFlag  [AUTO-TRANSLATED:77a18970]
    // Search for the first PsHeaderFlag in rtp
    auto ps_header_flag_ptr = findPsHeaderFlag(data, len);
    if (!ps_header_flag_ptr) {
        InfoL << "rtp搜索flag失败，丢弃rtp数据为：" << len;
        return nullptr;
    }

    auto rtp_ptr = ps_header_flag_ptr - kPSHeaderOffset;
    _search_rtp_finished = true;
    if (rtp_ptr == data) {
        // 停止搜索rtp，否则会进入死循环  [AUTO-TRANSLATED:319eefa7]
        // Stop searching for rtp, otherwise it will enter an infinite loop
        _search_rtp = false;
    }
    InfoL << "rtp搜索flag成功，tcp上下文恢复成功，丢弃的rtp残余数据为：" << rtp_ptr - data;

    // TODO or Not ? 更新设置ssrc  [AUTO-TRANSLATED:9c21db0a]
    // TODO or Not ? Update setting ssrc
    uint32_t rtp_ssrc = 0;
    getSSRC(rtp_ptr + 2, len, rtp_ssrc);
    _ssrc = rtp_ssrc;
    InfoL << "设置_ssrc为：" << _ssrc;
    // RtpServer::updateSSRC(uint32_t ssrc)
    return rtp_ptr;
}

} // namespace mediakit
#endif // defined(ENABLE_RTPPROXY)
