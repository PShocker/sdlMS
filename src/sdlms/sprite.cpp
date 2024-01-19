#include "sdlms/sprite.hpp"

Sprite::Sprite(SDL_Texture *texture, SDL_FRect rect, int format) : _texture(texture),
                                                                   _rect(rect),
                                                                   _format(format)
{
    _camera = Camera::current();
    _graphics = Graphics::current();
}

Sprite::~Sprite()
{
}

void Sprite::draw()
{
    auto fr = rect();
    fr.x -= _camera->viewport.x;
    fr.y -= _camera->viewport.y;
    _graphics->blitSurface(_texture, NULL, &fr);
}

SDL_FRect Sprite::rect()
{
    return _rect;
}
