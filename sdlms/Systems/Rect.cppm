module;

#include <SDL3/SDL.h>

export module systems:rect;

import components;

export SDL_FRect real_rect(SDL_FRect rect, Transform *tr);
