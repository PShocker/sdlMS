#pragma once

#include <SDL3/SDL.h>
#include "Systems/UI.h"

struct Keyboard
{
    static void init();
    enum Action : uint8_t
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        JUMP,
        SIT,
        PICK,
        ATTACK
    };
    static inline std::map<uint8_t, SDL_Scancode> action;
    static inline std::map<SDL_Scancode, int> skill;
    static inline std::map<SDL_Scancode, std::u16string> item;
    static inline std::map<SDL_Scancode, uint8_t> ui;
    static inline std::map<SDL_Scancode, bool> press;
};