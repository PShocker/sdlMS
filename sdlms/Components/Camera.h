#pragma once
#include "Component.h"
#include <SDL2/SDL.h>

//
class Camera : public Component
{
public:
    Camera(int x, int y, float w, float h);
    constexpr auto get_x() { return x; }
    constexpr auto get_y() { return y; }
    constexpr auto get_w() { return w; }
    constexpr auto get_h() { return h; }

    void set_x(const int val) { x = val; }
    void set_y(const int val) { y = val; }
    void set_w(const float val) { w = val; }
    void set_h(const float val) { h = val; }

private:
    int x;
    int y;
    float w;
    float h;
};