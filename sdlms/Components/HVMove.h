#pragma once
#include <optional>
#include "Component.h"

// 表示左右运动的属性
class HVMove : public Component
{
public:
    HVMove(int rx, int ry);
    constexpr auto get_rx() { return rx; }
    constexpr auto get_ry() { return ry; }

private:
    int rx;
    int ry;
};