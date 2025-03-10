#pragma once

#include "Sprite.h"

#include <SDL3/SDL.h>
#include <optional>
#include <list>

struct Damage
{
    static inline SpriteWarp *red[10];
    static inline SpriteWarp *violet[10];
    static inline SpriteWarp *cri[10];
    static inline SpriteWarp *blue[10];
    static void init();

    struct Info
    {
        int damage;
        float alpha;
        enum Type : uint8_t
        {
            Red = 0,
            Violet = 1,
            Cri = 2,
            Blue = 3
        };
        uint8_t type; // 0:red,1:violet,2:cri,3:blue

        unsigned int delay;
        float x;
        float y;
        SDL_FPoint point;
    };

    std::list<Info> damage_list;
    char index = 0;

    static void push(Damage *dam, int damage, uint8_t type, SDL_FPoint point);
};
