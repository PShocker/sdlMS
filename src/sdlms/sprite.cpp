#include "sdlms/sprite.hpp"
#include "sprite.hpp"

Sprite::Sprite(const std::vector<uint8_t> &raw_data,
               SDL_FRect rect,
               unsigned int format, int flip) : _rect(rect),
                                                _format(format), _flip(flip)
{
    switch (format)
    {
    case 1:
        format = SDL_PIXELFORMAT_ARGB4444;
        _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
        SDL_UpdateTexture(_texture, NULL, raw_data.data(), rect.w * sizeof(Uint16));
        break;
    case 2:
        format = SDL_PIXELFORMAT_ARGB8888;
        _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
        SDL_UpdateTexture(_texture, NULL, raw_data.data(), rect.w * sizeof(Uint32));
        break;
    default:
        break;
    }
    SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

Sprite::Sprite(SDL_Texture *texture, SDL_FRect rect, int flip) : _texture(texture), _rect(rect), _flip(flip)
{
}

void Sprite::draw()
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    fr.x -= camera->viewport.x;
    fr.y -= camera->viewport.y;
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
