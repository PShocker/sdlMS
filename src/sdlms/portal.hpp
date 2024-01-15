#ifndef HEADER_SDLMS_PORTAL
#define HEADER_SDLMS_PORTAL

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"
#include "sdlms/dynamicsprite.hpp"

class Portal : public DynamicSprite
{
public:
    Portal(std::variant<Sprite, AnimatedSprite> dynamicsprite,
           int type,
           std::u16string url);
    void update(int elapsedTime);

public:
    int _type;
    std::u16string _url;

private:
    enum Type
    {
        EDITOR,
        GAME
    };
};

#endif