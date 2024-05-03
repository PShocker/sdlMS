#pragma once
#include "Component.h"
#include <optional>
class RigidLine : public Component
{
public:
    RigidLine(SDL_FPoint m, SDL_FPoint n);
    constexpr auto get_m() { return m; }
    constexpr auto get_n() { return n; }
    std::optional<float> get_y(float x);

private:
    SDL_FPoint m;
    SDL_FPoint n;

private:
    std::optional<float> k = std::nullopt;
    std::optional<float> intercept = std::nullopt;
};
