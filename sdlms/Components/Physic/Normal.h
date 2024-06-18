#pragma once
#include "../Component.h"
#include <optional>

// 正常物理状态,非水下,飞翔
class Normal : public Component
{
public:
    Normal(){};

public:
    float hspeed = 0.0;
    std::optional<SDL_FPoint> hspeed_limit = SDL_FPoint{-125.0f, 125.0f};
    float vspeed = 0.0;
    std::optional<SDL_FPoint> vspeed_limit = SDL_FPoint{-5000.0f, 670.0f};
    float hforce = 0.0;
    float vforce = 0.0;
    float hacc = 0.0;
    float vacc = 0.0;
    enum : uint8_t
    {
        Ground,
        Air,
        Climb,
    };
    uint8_t type = Air;

    enum : uint8_t
    {
        None,
        Up,
        Left,
        Down,
        Right,
    };
    uint8_t vkey = None; // 上下按键
    uint8_t hkey = None; // 左右按键
    bool lalt = false;
    bool lctrl = false;
};