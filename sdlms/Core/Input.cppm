module;
#include <SDL3/SDL.h>
export module core:input;

export struct Input
{
    static const inline bool *state = SDL_GetKeyboardState(nullptr);
};
