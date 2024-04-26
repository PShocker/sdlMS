#pragma once
#include "Component.h"
#include "wz/Property.hpp"

class Sound : public Component
{
public:
    static Sound *load_sound(wz::Node *node);
    static inline std::map<wz::Node *, Sound *> sound_map;

private:
    Sound(wz::Node *node);
    Sound(){};

public:
    const auto get_pcm_data() { return pcm_data; }
    const auto get_freq() { return freq; }
    const auto get_offset() { return offset; }
    const auto get_delay() { return delay; }

    void set_offset(unsigned int val) { offset = val; }

public:
    std::vector<uint8_t> pcm_data;
    unsigned int freq;
    unsigned int offset; // 记录当前播放的位置
    unsigned int delay;  // 播放声音后下次播放的间隔,若为-1,则表示播放1次
};
