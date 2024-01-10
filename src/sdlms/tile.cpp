#include "sdlms/tile.hpp"

Tile::Tile(SDL_Texture *texture, SDL_Rect *rect, int format, int layer, int z) : _texture(texture),
                                                                                 _rect(rect),
                                                                                 _format(format),
                                                                                 _layer(layer),
                                                                                 _z(z)
{
}
