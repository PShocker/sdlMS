#pragma once
#include "Component.h"

class HVTile : public Component
{
public:
    HVTile(int cx, int cy);

private:
    int cx;
    int cy;
};