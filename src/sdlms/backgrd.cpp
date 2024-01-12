#include "sdlms/backgrd.hpp"

BackGrd::BackGrd(std::variant<Sprite, AnimatedSprite> backgrd,
                 int type, int front, std::u16string url) : _backgrd(backgrd),
                                                            _type(type), _front(front),
                                                            _url(url)
{
}

void BackGrd::update(int elapsedTime)
{
    auto htile = 0;
    auto vtile = 0;

    auto hspeed = 0;
    auto vspeed = 0;
    switch (_type)
    {
    case Type::HTILED:
    case Type::HMOVEA:
        htile = 1;
        break;
    case Type::VTILED:
    case Type::VMOVEA:
        vtile = 1;
        break;
    case Type::TILED:
    case Type::HMOVEB:
    case Type::VMOVEB:
        htile = 1;
        vtile = 1;
        break;
    }
    // switch (_type)
    // {
    // case Type::HMOVEA:
    // case Type::HMOVEB:
    //     hspeed = rx / 16;
    //     break;
    // case Type::VMOVEA:
    // case Type::VMOVEB:
    //     vspeed = ry / 16;
    //     break;
    // }
}

void BackGrd::draw()
{
    if (std::holds_alternative<Sprite>(_backgrd))
    {
        std::get<Sprite>(_backgrd).draw();
    }
    else if (std::holds_alternative<AnimatedSprite>(_backgrd))
    {
        std::get<AnimatedSprite>(_backgrd).draw();
    }
}