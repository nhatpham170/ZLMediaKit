#include "JT1078Process.h"
#include <Network/Buffer.h>
namespace mediakit {
    std::vector<toolkit::Buffer::Ptr> * nalus;
    std::vector<toolkit::Buffer::Ptr> * packages;    
    std::vector<char> * reverse = new std::vector<char>();
    bool JT1078Process::isJT1078(const toolkit::Buffer::Ptr &buff) {
        const char *buf = buff->data();

        return (uint8_t)buf[0] == 0x30 && (uint8_t)buf[1] == 0x31 && (uint8_t)buf[2] == 0x63 && (uint8_t)buf[3] == 0x64;
    }
    bool JT1078Process::parsePackage(const toolkit::Buffer::Ptr &data) {                
        // merge data
        char *_data = data->data();
        int _data_size = data->size();        
        for (size_t i = 0; i < _data_size; i++) {
            reverse->push_back(_data[i]);
        }
        // process find package 
        char *buf = reverse->data();
        int size = reverse->size();
        int seek = 0;
        int remain = size - seek;
        int start = -1;
        int end = -1;
        packages = new std::vector<toolkit::Buffer::Ptr>();        
        while (remain > 50) {
            remain = size - seek;
            if ((uint8_t)buf[seek] == 0x30 && (uint8_t)buf[seek + 1] == 0x31 && (uint8_t)buf[seek + 2] == 0x63 && (uint8_t)buf[seek+3] == 0x64) {                
                if (start >= 0) {
                    // end
                } else {
                    start = seek;
                }                
            }
            seek += 1;
        }

        return false;
    }
    void JT1078Process::parseNalu(const toolkit::Buffer::Ptr &buff) {}
    char *JT1078Process::inputNalu(const toolkit::Buffer::Ptr &buff) {
        return nullptr;
    }
    char *JT1078Process::h264Rtp(const toolkit::Buffer::Ptr &buff) {
        return nullptr;
    }
    char *JT1078Process::h265Rtp(const toolkit::Buffer::Ptr &buff) {
        return nullptr;
    }
} // namespace mediakit