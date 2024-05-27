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
    std::vector<std::tuple<std::optional<SDL_FPoint>, std::optional<SDL_FPoint>, std::variant<Sprite *, AnimatedSprite *>>> list;

public:
    DistanceSprite();
    ~DistanceSprite();
    void add(std::optional<SDL_FPoint> m, std::optional<SDL_FPoint> n, std::variant<Sprite *, AnimatedSprite *> disspr);
    constexpr auto &get_list() { return list; }
};
