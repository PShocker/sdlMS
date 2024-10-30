module;

#include <SDL3/SDL.h>

export module systems:collision;

import components;

export bool collision(SDL_FRect m_rect, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr);
