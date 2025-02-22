#pragma once

#include "Sprite.h"

#include <vector>
#include <SDL3/SDL.h>
#include <optional>

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
        int index;
        char type; // 0:red,1:violet,2:cri,3:blue
    };

    std::vector<Info> damage;
    std::optional<SDL_FPoint> head;
};
