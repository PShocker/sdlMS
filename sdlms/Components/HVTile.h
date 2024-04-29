#pragma once
#include <optional>
#include "Component.h"

// 平铺属性
class HVTile : public Component
{
public:
    HVTile(std::optional<int> cx, std::optional<int> cy);
    constexpr auto get_cx() { return cx; }
    constexpr auto get_cy() { return cy; }

private:
    std::optional<int> cx; // 当值为空表示该方向不做平铺
    std::optional<int> cy;
};