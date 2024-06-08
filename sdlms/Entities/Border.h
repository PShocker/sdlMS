#pragma once
#include "Entity.h"
#include "wz/Property.hpp"
#include <optional>

class Border : public Entity
{
public:
    Border(wz::Node *node, World *world);
    ~Border();
    constexpr auto get_left() { return left; }
    constexpr auto get_right() { return right; }
    constexpr auto get_top() { return top; }
    constexpr auto get_bottom() { return bottom; }

private:
    std::optional<float> left = std::nullopt;
    std::optional<float> right = std::nullopt;
    std::optional<float> top = std::nullopt;
    std::optional<float> bottom = std::nullopt;
};
