#pragma once

#include "wz/Property.hpp"
#include <optional>

struct FootHold
{
    int id;
    int page;
    int zmass;
    FootHold *prev;
    FootHold *next;

    int x1;
    int y1;
    int x2;
    int y2;

    int l;
    int r;
    int t;
    int b;

    bool forbidFallDown = false;

    std::optional<float> k = std::nullopt; // 斜率,若k值不存在,可以判断是否是墙面
    std::optional<float> intercept = std::nullopt;

    std::optional<float> get_x(float y);
    std::optional<float> get_y(float x);

    FootHold() = default;
};

struct WallFootHold
{
};
struct FloorFootHold
{
};