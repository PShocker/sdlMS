#pragma once

#include "Sprite.h"
#include "Core/Window.h"
#include <SDL3/SDL.h>
#include <optional>
#include <list>

struct Damage
{
    enum Type : uint8_t
    {
        Red = 0,
        Violet = 1,
        Cri = 2,
        Blue = 3
    };

    struct Wrap
    {
        int damage;
        float alpha;
        uint8_t type; // 0:red,1:violet,2:cri,3:blue
        unsigned int delay;
        SDL_FPoint point;
    };

    std::list<Wrap> damages;
    char index = 0;

    static inline Sprite::Wrap *red[10];
    static inline Sprite::Wrap *violet[10];
    static inline Sprite::Wrap *cri[11];
    static inline Sprite::Wrap *blue[10];

    static void init();

    static void push(Damage *dam, int damage, uint8_t type, SDL_FPoint point, unsigned int delay = Window::dt_now);
};
