#include "sdlms/tile.hpp"

Tile::Tile(SDL_Texture *texture, SDL_FRect *rect, int format, int layer, int z) : Sprite(texture, rect, format),
                                                                                 _layer(layer),
                                                                                 _z(z)
{
}
