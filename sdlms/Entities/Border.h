#pragma once
#include "Entity.h"
#include "wz/Property.hpp"

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
    float left = 0;
    float right = 0;
    float top = 0;
    float bottom = 0;
};
