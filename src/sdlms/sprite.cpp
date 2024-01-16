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
    SDL_FRect rect{_rect.x, _rect.y, _rect.w, _rect.h};
    rect.x -= _camera->viewport.x;
    rect.y -= _camera->viewport.y;
    _graphics->blitSurface(_texture, NULL, &rect);
}
