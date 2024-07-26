module;

#include <SDL3/SDL.h>

export module components:attack;

export struct Attack
{
    SDL_Rect rect;
    int mobCount;
    int attackCount;
};