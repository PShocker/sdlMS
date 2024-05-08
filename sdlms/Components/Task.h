#pragma once
#include "Component.h"
#include <functional>

class Task : public Component
{
public:
    Task(std::function<void(Entity *ent, World &world)> func, int delay);

public:
    int delay;
    std::function<void(Entity *ent, World &world)> func;
};