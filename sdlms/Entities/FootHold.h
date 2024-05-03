#pragma once
#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

class FootHold : public Entity
{
public:
    FootHold(wz::Node *node, int id, int page, int zmass, World *world);

public:
    constexpr auto get_page() { return page; }

private:
    // fh的附加属性
    int id;
    int page;
    int zmass;
    int prev;
    int next;
};
