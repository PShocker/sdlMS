#pragma once
#include "../Component.h"

// 正常物理状态,非水下,飞翔
class Normal : public Component
{
public:
    Normal(){};

public:
    float hspeed = 0.0;
    float vspeed = 0.0;
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
        Down,
    };
    uint8_t want_climb = None; // 是否有爬梯子或绳子的倾向
};