#include "DistanceSprite.h"

DistanceSprite::DistanceSprite()
{
}

DistanceSprite::~DistanceSprite()
{
}

void DistanceSprite::add(std::optional<SDL_FPoint> m, std::optional<SDL_FPoint> n, std::variant<Sprite *, AnimatedSprite *> disspr)
{
    list.push_back({m, n, disspr});
}
