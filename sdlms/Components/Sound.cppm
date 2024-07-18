module;
#include "wz/Property.hpp"

export module components:sound;

export struct Sound
{
    std::vector<uint8_t> *pcm_data;
    unsigned int freq = 0;
    unsigned int offset = 0; // 记录当前播放的位置
    unsigned int delay = 0;  // 播放声音后下次播放的间隔,若为-1,则表示播放1次
    bool play = false;       // 当前是否播放
    Sound() = default;
};
