#include "RigidLine.h"
#include <algorithm>

RigidLine::RigidLine(SDL_FPoint m, SDL_FPoint n) : m(m), n(n)
{
    auto [x1, y1] = m;
    auto [x2, y2] = n;
    if (x1 != x2)
    {
        // 斜坡
        k = ((float)y2 - (float)y1) / ((float)x2 - (float)x1);
        intercept = y1 - k.value() * x1;
    }
}

std::optional<float> RigidLine::get_y(float x)
{
    if (x == std::clamp(x, (float)m.x, (float)n.x))
    {
        if (k.has_value())
        {
            return k.value() * x + intercept.value();
        }
    }
    return std::nullopt;
}
