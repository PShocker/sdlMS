#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"
#include "Components/Sprite.h"

class ChatBalloon : public Entity
{
public:
    ChatBalloon(int width, int height, const std::u16string &val);
    ~ChatBalloon();
    int get_width();
    int get_height();

public:
    Sprite *spr;
};
