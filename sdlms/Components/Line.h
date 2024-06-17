#pragma once
#include "Component.h"
#include <optional>
#include <cmath>
// 线
class Line : public Component
{
public:
    Line(SDL_FPoint m, SDL_FPoint n);
    constexpr auto get_m() { return m; }
    constexpr auto get_n() { return n; }
    auto get_min_x() { return std::fmin(m.x, n.x); }
    auto get_max_x() { return std::fmax(m.x, n.x); }
    auto get_min_y() { return std::fmin(m.y, n.y); }
    auto get_max_y() { return std::fmax(m.y, n.y); }
    constexpr auto get_k() { return k; }
    std::optional<float> get_y(float x);
    std::optional<float> get_x(float y);

private:
    SDL_FPoint m;
    SDL_FPoint n;

private:
    std::optional<float> k = std::nullopt; // 斜率,若k值不存在,可以判断是否是墙面
    std::optional<float> intercept = std::nullopt;
};
