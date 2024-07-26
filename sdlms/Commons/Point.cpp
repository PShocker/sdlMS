module;

#include <SDL3/SDL.h>
#include <optional>

module commons;

SDL_FPoint operator+(const SDL_FPoint &m, const SDL_FPoint &n)
{
    return {m.x + n.x, m.y + n.y};
}

SDL_FPoint operator-(const SDL_FPoint &m, const SDL_FPoint &n)
{
    return {m.x - n.x, m.y - n.y};
}

std::optional<SDL_FPoint> intersect(SDL_FPoint p1, SDL_FPoint p2, SDL_FPoint p3, SDL_FPoint p4)
{
    // 快速排斥实验
    if ((p1.x > p2.x ? p1.x : p2.x) < (p3.x < p4.x ? p3.x : p4.x) ||
        (p1.y > p2.y ? p1.y : p2.y) < (p3.y < p4.y ? p3.y : p4.y) ||
        (p3.x > p4.x ? p3.x : p4.x) < (p1.x < p2.x ? p1.x : p2.x) ||
        (p3.y > p4.y ? p3.y : p4.y) < (p1.y < p2.y ? p1.y : p2.y))
    {
        return std::nullopt;
    }
    // 跨立实验
    if ((((p1.x - p3.x) * (p4.y - p3.y) - (p1.y - p3.y) * (p4.x - p3.x)) *
         ((p2.x - p3.x) * (p4.y - p3.y) - (p2.y - p3.y) * (p4.x - p3.x))) > 0 ||
        (((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) *
         ((p4.x - p1.x) * (p2.y - p1.y) - (p4.y - p1.y) * (p2.x - p1.x))) > 0)
    {
        return std::nullopt;
    }

    auto x = ((p1.y - p3.y) * (p2.x - p1.x) * (p4.x - p3.x) + p3.x * (p4.y - p3.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (p4.x - p3.x)) / ((p4.x - p3.x) * (p1.y - p2.y) - (p2.x - p1.x) * (p3.y - p4.y));
    auto y = (p2.y * (p1.x - p2.x) * (p4.y - p3.y) + (p4.x - p2.x) * (p4.y - p3.y) * (p1.y - p2.y) - p4.y * (p3.x - p4.x) * (p2.y - p1.y)) / ((p1.x - p2.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p3.x - p4.x));

    return SDL_FPoint{x, y};
}
