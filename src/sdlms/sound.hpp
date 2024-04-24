#ifndef HEADER_SDLMS_SOUND
#define HEADER_SDLMS_SOUND

#include <cstdint>
#include <vector>
#include <optional>

#include <SDL2/SDL.h>
#include "template/currenton.hpp"
#include "wz/Property.hpp"

class Sound : public Currenton<Sound>
{
public:
    Sound();
    Sound(std::vector<uint8_t> data, int freq);

    static std::optional<Sound> load_bgm(int mapId);
    static std::optional<Sound> load(std::u16string url);
    int play(std::tuple<std::vector<uint8_t>, int> data);
    std::tuple<std::vector<uint8_t>, int> _bgm; // pcm流,采样率

    static inline std::map<std::u16string, Sound> sound_map;

public:
    std::vector<uint8_t> _data; // 音频流
    int _freq;                  // 采样率
};

#endif
