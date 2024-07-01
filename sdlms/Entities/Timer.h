#pragma once
#include "Entity.h"
#include <functional>

class Timer : public Entity
{
public:
    Timer(const std::function<Uint32(Uint32, void *)> &callback, int time);

public:
    int time = 0;
    std::function<Uint32(Uint32, void *)> callback;

};