#pragma once
#include "Component.h"

// 随机产生按键输出组件
class RandomInput : public Component
{
private:
    unsigned int count = 0;
    unsigned int tick = 0;

public:
    RandomInput(int val = 60);
    auto get_count() { return count; };
    auto get_tick() { return tick; };
    void set_count(const unsigned int value) { count = value; };
    void set_tick(const unsigned int value) { tick = value; };
};