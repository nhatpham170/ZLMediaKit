#include <Network/Buffer.h>
namespace mediakit {
class JT1078Process {        
public:
    std::string TYPE = "JT1078";
    std::string imei;        
    uint8_t last_type = 0;
    uint64_t time = 0;
    uint8_t procol_media;

    static bool isJT1078(const toolkit::Buffer::Ptr &buff);
    bool parsePackage(const toolkit::Buffer::Ptr &buff);
    void parseNalu(const toolkit::Buffer::Ptr &buff);
    char *inputNalu(const toolkit::Buffer::Ptr &buff);
    char *h264Rtp(const toolkit::Buffer::Ptr &buff);
    char *h265Rtp(const toolkit::Buffer::Ptr &buff);

private:
};

} // namespace mediakit