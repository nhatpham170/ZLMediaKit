#pragma once
#include <cstdint>
#include <string>
namespace mediakit {
class JT1078Package {
public:
    const int FH = 0x30316364;
    int label1 = 0;
    int label2 = 0;
    std::string header = "";
    std::string sqe = "";
    int sn = 0;
    std::string imei = "";
    std::string channel = "";
    std::string body = "";
    int ch = 0;
    int label3 = 0;
    int dataType = 0;
    int subPackageType = 0;
    uint64_t timestamp = 0;
    int iframeInterval = 0;
    int frameInterval = 0;
    int length = 0;
    JT1078Package() {}
    void GetKey();
    enum DataType {
        VideoI = 0,
        VideoP = 1,
        VideoB = 2,
        Audio = 3,
        TransparentData = 4,
    };
    enum SubPackageType { Fixed = 0, FirstPacket = 1, LastPacket = 2, SubPacket = 3 };
};
} // namespace mediakit
