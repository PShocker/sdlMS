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
        char type; // 0:red,1:violet,2:cri,3:blue

        unsigned int delay;
        float x;
        float y;
    };

    std::list<Info> damage_list;
    std::optional<SDL_FPoint> head;
    char index = 0;

    static void push(Damage *dam, int damage, char type);
};
