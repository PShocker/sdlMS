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
        Left,
        Down,
        Right,
    };
    uint8_t vkey = None; // 上下按键
    bool want_prone = false;   // 是否有趴下的倾向
    bool want_fall = false;    // 是否有下跳的倾向
    bool want_stand = false;   // 是否有stand的倾向
    bool want_jump = false;    // 是否有jump的倾向
    uint8_t hkey = None;    // 左右按键
};