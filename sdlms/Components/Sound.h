#pragma once

#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include <unordered_set>

extern SDL_AudioStream *audio_stream;

struct SoundWarp
{
    std::vector<uint8_t> pcm_data;

    static SoundWarp *load(wz::Node *node, bool caches = true);
    static void clean_up();

    SoundWarp(wz::Node *node);
    SoundWarp() = default;
};

struct Sound
{
    SoundWarp *souw = nullptr;
    Sound(wz::Node *node, int d = 0);
    Sound(const std::u16string &path, int d = 0);
    Sound() = default;

    unsigned int offset = 0; // 记录当前播放的位置
    unsigned int delay = 0;  // 记录当前播放是否延迟,数值表示延迟时间
    bool circulate = false;  // 当前是否循环
    bool bgm = false;        // 当前是否bgm

    static bool init();
    static void push(SoundWarp *souw, int delay = 0, int pos = -1);
    static void push(Sound sou, int pos = -1);
    static void remove(int pos);
    static Sound *at(int pos);
};
