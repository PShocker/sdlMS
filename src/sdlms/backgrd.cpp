#include "sdlms/backgrd.hpp"

BackGrd::BackGrd(std::variant<Sprite, AnimatedSprite> backgrd,
                 int type, int front,
                 int rx, int ry,
                 int cx, int cy,
                 int ani, std::u16string url) : _backgrd(backgrd),
                                                _type(type), _front(front),
                                                _rx(rx), _ry(ry),
                                                _cx(cx), _cy(cy),
                                                _ani(ani),
                                                _url(url)
{
}

void BackGrd::update(int elapsedTime)
{
    auto htile = 0; // 水平平铺
    auto vtile = 0; // 垂直平铺

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
    switch (_type)
    {
    case Type::HMOVEA:
    case Type::HMOVEB:
        hspeed = _rx / 16;
        break;
    case Type::VMOVEA:
    case Type::VMOVEB:
        vspeed = _ry / 16;
        break;
    }

    auto viewprot = Camera::current()->viewport;
    if (std::holds_alternative<Sprite>(_backgrd))
    {
        auto tile_cnt_x = 1;
        auto s = std::get<Sprite>(_backgrd);
        SDL_Point point = {s._rect->x, s._rect->y};
        if (htile > 0)
        {
            auto tile_start_right = int(point.x + s._rect->w - viewprot.x) % _cx;
            if (tile_start_right <= 0)
            {
                tile_start_right = tile_start_right + _cx;
            }
            tile_start_right = tile_start_right + viewprot.x;
            auto tile_start_left = tile_start_right - s._rect->w;
            if (tile_start_left >= viewprot.x + viewprot.w)
            {
                tile_cnt_x = 0;
            }
            else
            {
                tile_cnt_x = ceil((viewprot.x + viewprot.w - tile_start_left) / float(_cx));
                point.x = tile_start_left;
            }
            for (int i = 0; i < tile_cnt_x; i++)
            {
                SDL_Rect *rect = new SDL_Rect{point.x + i * _cx, s._rect->y, s._rect->w, s._rect->h};
                _backgrds.push_back(Sprite(s._texture, rect, SDL_PIXELFORMAT_ARGB4444));
            }
        }

        if (vtile > 0)
        {
        }
    }
}

void BackGrd::draw()
{
    for (auto it : _backgrds)
    {
        if (std::holds_alternative<Sprite>(it))
        {
            std::get<Sprite>(it).draw();
        }
        else if (std::holds_alternative<AnimatedSprite>(it))
        {
            std::get<AnimatedSprite>(it).draw();
        }
    }
}