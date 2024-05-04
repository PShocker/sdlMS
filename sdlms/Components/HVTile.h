#pragma once
#include "Component.h"

// 平铺属性
class HVTile : public Component
{
public:
    HVTile(int cx, int cy,bool htile, bool vtile);
    constexpr auto get_cx() { return cx; }
    constexpr auto get_cy() { return cy; }

public:
    int cx; // 当值为空表示该方向不做平铺
    int cy;
    bool htile = false;
    bool vtile = false;
};