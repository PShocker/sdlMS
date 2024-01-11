#include "sdlms/tile.hpp"

Tile::Tile(SDL_Texture *texture, SDL_Rect *rect, int format, int layer, int z) : _texture(texture),
                                                                                 _rect(rect),
                                                                                 _format(format),
                                                                                 _layer(layer),
                                                                                 _z(z)
{
    _camera = Camera::current();
    _graphics = Graphics::current();
}

void Tile::draw()
{
    SDL_Rect rect{_rect->x, _rect->y, _rect->w, _rect->h};
    rect.x -= _camera->viewport.x;
    _graphics->blitSurface(_texture, NULL, &rect);
}
