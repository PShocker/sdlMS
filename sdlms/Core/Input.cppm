module;
#include <SDL3/SDL.h>
export module core:input;

export struct Input
{
    static const inline Uint8 *state = SDL_GetKeyboardState(nullptr);
};
