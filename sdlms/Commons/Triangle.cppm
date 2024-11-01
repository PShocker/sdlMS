module;

#include <SDL3/SDL.h>

export module commons:triangle;

// 三角形结构
export struct Triangle
{
    SDL_FPoint vertex1;
    SDL_FPoint vertex2;
    SDL_FPoint vertex3;
};

// 判断矩形是否有点在三角形内
export bool RectIntersectTriangle(const SDL_FRect &rect, const Triangle &t);

// 判断点是否在三角形内
export bool PointInTriangle(const SDL_FPoint &pt, const Triangle &t);

// 计算矩形上的点距离另一个点的最近位置
export SDL_FPoint closestPointToRect(const SDL_FPoint &point, const SDL_FRect &rect);