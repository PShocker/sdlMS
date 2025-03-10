#pragma once

#include "AnimatedSprite.h"
#include "Sound.h"
#include <string>
#include <vector>
#include <unordered_map>

struct Reactor
{
    struct Info
    {
        struct Event
        {
            int type = -1;
            int state = -1;
        };
        AnimatedSprite init;
        AnimatedSprite hit;
        std::vector<Event> event;
    };

    std::u16string id;
    int reactorTime;
    std::unordered_map<int, Info> a;
    int index = 0;
    bool hit = false;

    std::unordered_map<int, SoundWarp *> sounds;

    Reactor() = default;
};