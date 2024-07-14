#include "Point.h"

SDL_FPoint operator+(const SDL_FPoint &m, const SDL_FPoint &n)
{
    return {m.x + n.x, m.y + n.y};
}

SDL_FPoint operator-(const SDL_FPoint &m, const SDL_FPoint &n)
{
    return {m.x - n.x, m.y - n.y};
}
