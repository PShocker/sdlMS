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
    int _type;
    int _front;
    int _rx;
    int _ry;
    int _cx;
    int _cy;
    int _ani;
    float _position_offset_x;
    float _position_offset_y;
    std::u16string _url;
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