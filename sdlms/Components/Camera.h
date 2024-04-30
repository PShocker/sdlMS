#pragma once
#include "Component.h"
#include <SDL2/SDL.h>

// 表示左右运动的属性
class Camera : public Component
{
public:
    Camera(float x, float y, float w, float h);
    constexpr auto get_rect() { return rect; }

private:
    SDL_FRect rect;
};