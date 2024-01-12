#ifndef HEADER_SDLMS_BACKGRD
#define HEADER_SDLMS_BACKGRD

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <variant>

#include "sdlms/animatedsprite.hpp"
#include "sdlms/sprite.hpp"

class BackGrd
{
public:
    BackGrd(std::variant<Sprite, AnimatedSprite> backgrd, int type, int front, std::u16string url);
    void draw();
    void update(int elapsedTime);


public:
    int _type;
    int _front;
    std::u16string _url;
    std::variant<Sprite, AnimatedSprite> _backgrd;

private:
    enum Type
    {
        NORMAL,
        HTILED,
        VTILED,
        TILED,
        HMOVEA,
        VMOVEA,
        HMOVEB,
        VMOVEB
    };
};
#endif