#ifndef HEADER_SDLMS_PORTAL
#define HEADER_SDLMS_PORTAL

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"

class Portal
{
public:
    Portal(std::variant<Sprite, AnimatedSprite> portal,
           int type,
           std::u16string url);
    void draw();
    void update(int elapsedTime);

public:
    std::variant<Sprite, AnimatedSprite> _portal;
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