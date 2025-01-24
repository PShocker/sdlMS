#include "FootHold.h"
#include <optional>

std::optional<float> FootHold::get_x(float y)
{
    if (y >= t && y <= b)
    {
        if (k.has_value())
        {
            return (y - intercept.value()) / k.value();
        }
    }
    return std::nullopt;
}

std::optional<float> FootHold::get_y(float x)
{
    if (x >= l && x <= r)
    {
        if (k.has_value())
        {
            return k.value() * x + intercept.value();
        }
    }
    return std::nullopt;
}
