#pragma once
#include "Component.h"
#include "wz/Property.hpp"

class Sound : public Component
{
public:
    static Sound *load(wz::Node *node);
    static inline std::map<wz::Node *, Sound *> sound_map;

private:
    Sound(wz::Node *node);
    Sound(){};

public:
    constexpr auto get_pcm_data() { return &pcm_data; }
    constexpr auto get_freq() { return freq; }
    constexpr auto get_offset() { return offset; }
    constexpr auto get_delay() { return delay; }
    constexpr auto get_play() { return play; }

    void set_offset(unsigned int val) { offset = val; }
    void set_play(bool val) { play = val; }

public:
    std::vector<uint8_t> pcm_data;
    unsigned int freq;
    unsigned int offset; // 记录当前播放的位置
    unsigned int delay;  // 播放声音后下次播放的间隔,若为-1,则表示播放1次
    bool play = false;   // 当前是否播放
};
