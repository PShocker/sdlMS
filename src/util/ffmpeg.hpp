#ifndef HEADER_UTIL_FFMPEG
#define HEADER_UTIL_FFMPEG

#include <vector>
#include <cstdint>

#include "util/currenton.hpp"

// 加载声音
namespace util
{
    class FFMPEG final : public Currenton<FFMPEG>
    {
    public:
        std::vector<uint8_t> decodeAudioToPCM(std::vector<uint8_t> data);
    };
}

#endif