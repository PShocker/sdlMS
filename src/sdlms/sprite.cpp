#include "sdlms/sprite.hpp"
#include "sprite.hpp"

Sprite::Sprite(const std::vector<uint8_t> &raw_data, SDL_FRect rect, unsigned int format) : _rect(rect),
                                                                                            _format(format)
{
    switch (format)
    {
    case 1:
        format = SDL_PIXELFORMAT_ARGB4444;
        break;
    case 2:
        format = SDL_PIXELFORMAT_ARGB8888;
        break;
    default:
        break;
    }
    _texture = SDL_CreateTexture(Graphics::current()->getRenderer(), format, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
    SDL_UpdateTexture(_texture, NULL, raw_data.data(), rect.w * sizeof(Uint16));
    SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

Sprite::Sprite(SDL_Texture *texture, SDL_FRect rect) : _texture(texture), _rect(rect)
{
}

void Sprite::draw()
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    fr.x -= camera->viewport.x;
    fr.y -= camera->viewport.y;
    graphics->blitSurface(_texture, NULL, &fr);
}

SDL_FRect Sprite::rect()
{
    return _rect;
}
