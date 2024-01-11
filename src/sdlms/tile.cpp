#include "sdlms/tile.hpp"

Tile::Tile(SDL_Texture *texture, SDL_Rect *rect, int format, int layer, int z) : Sprite(texture, rect, format),
                                                                                 _layer(layer),
                                                                                 _z(z)
{
}

void Tile::draw()
{
    SDL_Rect rect{_rect->x, _rect->y, _rect->w, _rect->h};
    rect.x -= _camera->viewport.x;
    rect.y -= _camera->viewport.y;
    _graphics->blitSurface(_texture, NULL, &rect);
}
