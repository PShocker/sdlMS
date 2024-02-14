#ifndef HEADER_UTIL_SOUND
#define HEADER_UTIL_SOUND

#include <vector>
#include <ranges>
#include "wz/Property.hpp"

#include "template/currenton.hpp"

// 加载声音
namespace util
{
    class SoundUtil final : public Currenton<SoundUtil>
    {
    public:
        std::vector<u8> load_sound(int mapId);
    };
}

#endif