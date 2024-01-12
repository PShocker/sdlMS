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
    BackGrd(std::variant<Sprite, AnimatedSprite> backgrd, int type,
            int front,
            int rx, int ry,
            int cx, int cy,
            int ani,
            std::u16string url);
    void draw();
    void update(int elapsedTime);

public:
    const int _type;
    const int _front;
    const int _rx;
    const int _ry;
    const int _cx;
    const int _cy;
    const int _ani;
    const std::u16string _url;
    std::variant<Sprite, AnimatedSprite> _backgrd;
    std::vector<std::variant<Sprite, AnimatedSprite>> _backgrds;

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