module;

#include <SDL3/SDL.h>

export module components:trap;
import :animatedsprite;

export struct Trap
{
    int damage = 5;

    Trap(int damage);

    SDL_FRect rect(AnimatedSprite *aspr);
};
