#pragma once

#include "Components/Component.h"
#include <vector>
#include <tuple>
#include <variant>
#include <optional>

#include "Sprite.h"
#include "AnimatedSprite.h"

class DistanceSprite : public Component
{
private:
    std::vector<std::optional<std::variant<Sprite *, AnimatedSprite *>>> dissprs;
    std::vector<std::optional<SDL_FPoint>> hs; // 水平范围
    std::vector<std::optional<SDL_FPoint>> vs; // 垂直范围

public:
    DistanceSprite();
    ~DistanceSprite();
    void add(std::optional<SDL_FPoint> h, std::optional<SDL_FPoint> v, std::optional<std::variant<Sprite *, AnimatedSprite *>> disspr);
    constexpr auto &get_dissprs() { return dissprs; }
    constexpr auto &get_hs() { return hs; }
    constexpr auto &get_vs() { return vs; }
};
