#pragma once

#include <SDL3/SDL.h>
#include "entt/entt.hpp"
#include "Components/Components.h"
#include "Commons/Commons.h"

bool collision(SDL_FRect m_rect, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr);

bool collision(Triangle t, Transform *t_tr, SDL_FRect n_rect, Transform *n_tr);