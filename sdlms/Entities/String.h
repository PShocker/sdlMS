#pragma once

#include "Entity.h"
#include "wz/Property.hpp"
#include "Core/World.h"

#include "Core/FreeType.h"

class String : public Entity
{
public:
    String(const std::u16string &s, SDL_Color color = {255, 255, 255, 255}, int size = 12);
    ~String();
};
