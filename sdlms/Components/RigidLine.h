#pragma once
#include "Component.h"
#include "Core/Line.h"
// 可碰撞的线,通常是fh
class RigidLine : public Component
{
public:
    RigidLine(SDL_FPoint m, SDL_FPoint n);
    ~RigidLine();
    constexpr auto get_m() { return line->get_m(); }
    constexpr auto get_n() { return line->get_n(); }
    constexpr auto get_min_x() { return line->get_min_x(); }
    constexpr auto get_max_x() { return line->get_max_x(); }
    constexpr auto get_min_y() { return line->get_min_y(); }
    constexpr auto get_max_y() { return line->get_max_y(); }
    constexpr auto get_line() { return line; }

public:
    Line *line;
};
