#pragma once

#include <SDL3/SDL.h>
#include "Components/Transform.h"

SDL_FRect real_rect(SDL_FRect rect, Transform *tr);
