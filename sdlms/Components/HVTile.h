#pragma once
#include "Component.h"

// 平铺属性
class HVTile : public Component
{
public:
    HVTile(int cx, int cy, bool htile, bool vtile);
    constexpr auto get_cx() { return cx; }
    constexpr auto get_cy() { return cy; }

public:
    // 平铺间隔
    int cx;
    int cy;
    // 是否平铺
    bool htile = false;
    bool vtile = false;
};