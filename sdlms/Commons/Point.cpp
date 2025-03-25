#include "Point.h"

#include <SDL3/SDL.h>
#include <optional>
#include <cmath>
#include <math.h>
#include <numbers>

SDL_FPoint operator+(const SDL_FPoint &m, const SDL_FPoint &n)
{
    return {m.x + n.x, m.y + n.y};
}

SDL_FPoint operator-(const SDL_FPoint &m, const SDL_FPoint &n)
{
    return {m.x - n.x, m.y - n.y};
}

std::optional<SDL_FPoint> intersect(const SDL_FPoint &p1, const SDL_FPoint &p2, const SDL_FPoint &p3, const SDL_FPoint &p4)
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

// 辅助函数：计算两点角度
float calculate_angle(const SDL_FPoint &from, const SDL_FPoint &to)
{
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    if (dx == 0.0f)
    {
        return (dy > 0) ? 90.0f : (dy == 0) ? 0
                                            : 270.0f;
    }
    const float angle_rad = std::atan2(dy, dx);
    return static_cast<float>(angle_rad * (180.0 / std::numbers::pi));
}

// 辅助函数：计算两点平方距离（避免 sqrt）
float squared_distance(const SDL_FPoint &a, const SDL_FPoint &b)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

float distance(const SDL_FPoint &m, const SDL_FPoint &n)
{
    float dx = n.x - m.x;
    float dy = n.y - m.y;
    return std::sqrt(dx * dx + dy * dy);
}
