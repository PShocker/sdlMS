#include "sdlms/sprite.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/graphics.hpp"

Sprite::Sprite(const std::vector<uint8_t> &raw_data,
               SDL_FRect rect,
               unsigned int format, int flip) : _rect(rect),
                                                _format(format), _flip(flip),
                                                _raw_data(raw_data)
{
    switch (format)
    {
    case 1:
        format = SDL_PIXELFORMAT_ARGB4444;
        _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
        SDL_UpdateTexture(_texture, NULL, raw_data.data(), rect.w * sizeof(Uint16));
        SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
        break;
    case 2:
        format = SDL_PIXELFORMAT_ARGB8888;
        _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
        SDL_UpdateTexture(_texture, NULL, raw_data.data(), rect.w * sizeof(Uint32));
        SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
        break;
    case 513:
        format = SDL_PIXELFORMAT_RGB565;
        _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
        SDL_UpdateTexture(_texture, NULL, raw_data.data(), rect.w * sizeof(Uint16));
        break;
    default:
        break;
    }
}

Sprite::Sprite(SDL_Texture *texture, SDL_FRect rect, int flip) : _texture(texture), _rect(rect), _flip(flip)
{
}

void Sprite::draw()
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    fr.x -= camera->_viewport.x;
    fr.y -= camera->_viewport.y;
    if (_flip > 0) // 翻转
    {
        graphics->blitSurfaceEx(_texture, NULL, &fr, 0, 0, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        graphics->blitSurface(_texture, NULL, &fr);
    }
}

void Sprite::_draw()
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    if (_flip > 0) // 翻转
    {
        graphics->blitSurfaceEx(_texture, NULL, &fr, 0, 0, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        graphics->blitSurface(_texture, NULL, &fr);
    }
}

SDL_FRect Sprite::rect()
{
    return _rect;
}
