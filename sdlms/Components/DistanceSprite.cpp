#include "DistanceSprite.h"

DistanceSprite::DistanceSprite(){}
DistanceSprite::~DistanceSprite(){}

void DistanceSprite::add(std::optional<SDL_FPoint> h, std::optional<SDL_FPoint> v, std::optional<std::variant<Sprite *, AnimatedSprite *>> disspr)
{
    hs.push_back(h);
    vs.push_back(v);
    dissprs.push_back(disspr);
}


