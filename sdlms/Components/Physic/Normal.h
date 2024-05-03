#pragma once
#include "../Component.h"

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
        GROUND,
        AIR,
        CLIMB,
    };
    uint8_t type = AIR;
};