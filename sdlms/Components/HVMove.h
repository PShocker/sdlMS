#pragma once
#include "Component.h"

// 表示左右运动的属性
class HVMove : public Component
{
public:
    HVMove(int rx, int ry, bool hspeed, bool vspeed);
    constexpr auto get_rx() { return rx; }
    constexpr auto get_ry() { return ry; }
    constexpr auto get_offset_x() { return offset_x; }
    constexpr auto get_offset_y() { return offset_y; }
    constexpr auto get_hspeed() { return hspeed; }
    constexpr auto get_vspeed() { return vspeed; }
    void set_offset_x(const float val) { offset_x = val; }
    void set_offset_y(const float val) { offset_y = val; }

public:
    int rx;
    int ry;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    bool hspeed = false;
    bool vspeed = false;
};