#include "Rect.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

SDL_FRect real_rect(SDL_FRect rect, Transform *tr)
{
    rect.x += tr->position.x;
    rect.y += tr->position.y;
    if (tr->flip == 1)
    {
        rect.x += 2 * (tr->position.x - rect.x) - rect.w;
    }
    return rect;
}
