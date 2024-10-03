module;

#include "wz/Property.hpp"
#include <list>

export module components:sound;

export struct SoundWarp
{
    std::vector<uint8_t> pcm_data;
    SoundWarp(wz::Node *node);
    SoundWarp() = default;
    static SoundWarp *load(wz::Node *node);
};

export struct Sound
{
    SoundWarp *souw = nullptr;
    Sound(wz::Node *node);
    Sound() = default;

    unsigned int offset = 0; // 记录当前播放的位置
    bool circulate = false;  // 当前是否循环
    bool bgm = false;  // 当前是否bgm

    static bool init();
    static inline std::list<Sound> sound_list;
};
