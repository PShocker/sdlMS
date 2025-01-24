#include "Triangle.h"
#include <SDL3/SDL.h>

bool RectIntersectTriangle(const SDL_FRect &rect, const Triangle &tri)
{
    SDL_FPoint points[4] = {
        {rect.x, rect.y},
        {rect.x + rect.w, rect.y},
        {rect.x, rect.y + rect.h},
        {rect.x + rect.w, rect.y + rect.h}};

    // 检查矩形的每个角点是否在三角形内
    for (const SDL_FPoint &pt : points)
    {
        if (PointInTriangle(pt, tri))
        {
            return true; // 有一个点在三角形内
        }
    }
    return false; // 所有点都不在三角形内
}

bool PointInTriangle(const SDL_FPoint &pt, const Triangle &tri)
{
    auto &[v1, v2, v3] = tri;
    float area = 0.5f * (-v2.y * v3.x + v1.y * (-v2.x + v3.x) + v1.x * (v2.y - v3.y) + v2.x * v3.y);
    float s = 1 / (2 * area) * (v1.y * v3.x - v1.x * v3.y + (v3.y - v1.y) * pt.x + (v1.x - v3.x) * pt.y);
    float t = 1 / (2 * area) * (v1.x * v2.y - v1.y * v2.x + (v1.y - v2.y) * pt.x + (v2.x - v1.x) * pt.y);
    return s >= 0 && t >= 0 && (s + t <= 1);
}

SDL_FPoint closestPointToRect(const SDL_FPoint &point, const SDL_FRect &rect)
{
    SDL_FPoint closest;
    // 确定最近点的 x 坐标
    if (point.x < rect.x)
    {
        closest.x = rect.x; // 点在矩形的左侧
    }
    else if (point.x > rect.x + rect.w)
    {
        closest.x = rect.x + rect.w; // 点在矩形的右侧
    }
    else
    {
        closest.x = point.x; // 点在矩形的范围内
    }
    // 确定最近点的 y 坐标
    if (point.y < rect.y)
    {
        closest.y = rect.y; // 点在矩形的上方
    }
    else if (point.y > rect.y + rect.h)
    {
        closest.y = rect.y + rect.h; // 点在矩形的下方
    }
    else
    {
        closest.y = point.y; // 点在矩形的范围内
    }
    return closest;
}
