#pragma once
#include <SDL2/SDL.h>
#include <optional>

// 线
class Line
{
public:
    Line(SDL_FPoint m, SDL_FPoint n);
    constexpr auto get_m() { return m; }
    constexpr auto get_n() { return n; }
    constexpr auto get_min_x() { return std::min(m.x, n.x); }
    constexpr auto get_max_x() { return std::max(m.x, n.x); }
    constexpr auto get_min_y() { return std::min(m.y, n.y); }
    constexpr auto get_max_y() { return std::max(m.y, n.y); }
    constexpr auto get_k() { return k; }
    std::optional<float> get_y(float x);

protected:
    SDL_FPoint m;
    SDL_FPoint n;

protected:
    std::optional<float> k = std::nullopt;
    std::optional<float> intercept = std::nullopt;
};
