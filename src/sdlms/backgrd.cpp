#include "sdlms/backgrd.hpp"

BackGrd::BackGrd(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect, std::vector<int> delay,
                 std::vector<int> format, int type, int front,
                 int frameIndex, int frameSize,
                 std::u16string url) : _texture(texture), _rect(rect),
                                       _delay(delay), _format(format),
                                       _type(type), _front(front),
                                       _frameIndex(frameIndex),
                                       _frameSize(frameSize),
                                       _frameTime(0),
                                       _url(url)
{
    _camera = Camera::current();
    _graphics = Graphics::current();
}

void BackGrd::draw()
{
    SDL_Rect rect{_rect[_frameIndex]->x, _rect[_frameIndex]->y, _rect[_frameIndex]->w, _rect[_frameIndex]->h};
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
    rect.x -= _camera->viewport.x;
    rect.y -= _camera->viewport.y;
    _graphics->blitSurface(_texture[_frameIndex], NULL, &rect);
}