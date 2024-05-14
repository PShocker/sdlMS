#pragma once
#include "Component.h"
#include <functional>

class Task : public Component
{
public:
    Task(std::function<void(Entity *ent, World &world)> func, int delay);
    const auto get_delay() { return delay; }
    const auto get_func() { return func; }

public:
    int delay;
    std::function<void(Entity *ent, World &world)> func;
};