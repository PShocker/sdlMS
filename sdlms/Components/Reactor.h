#pragma once

#include "AnimatedSprite.h"
#include "Sound.h"
#include <string>
#include <vector>
#include <unordered_map>

struct Reactor
{
    struct Wrap
    {
        AnimatedSprite init;
        AnimatedSprite hit;
        // type,state
        std::vector<std::pair<int, int>> event;
    };

    std::u16string id;
    int reactorTime;
    std::unordered_map<int, Wrap> a;
    int index = 0;
    bool hit = false;

    std::unordered_map<int, Sound::Wrap *> sounds;

    Reactor() = default;
};