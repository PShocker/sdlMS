#include "sdlms/tile.hpp"

Tile::Tile(SDL_Texture *texture, SDL_FRect *rect, int format, int layer, int z) : texture(texture),
                                                                                 rect(rect),
                                                                                 format(format),
                                                                                 layer(layer),
                                                                                 z(z)
{
}
