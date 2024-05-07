#pragma once
#include "Component.h"
#include "Core/Line.h"

// 可爬行的线段,通常指梯子或绳子
class CrawlLine : public Component
{
public:
    CrawlLine(SDL_FPoint m, SDL_FPoint n);
    constexpr auto get_m() { return line->get_m(); }
    constexpr auto get_n() { return line->get_n(); }
    constexpr auto get_min_x() { return line->get_min_x(); }
    constexpr auto get_max_x() { return line->get_max_x(); }
    constexpr auto get_min_y() { return line->get_min_y(); }
    constexpr auto get_max_y() { return line->get_max_y(); }
    constexpr auto get_line() { return line; }

private:
    Line *line;
};