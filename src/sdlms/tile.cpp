#include "sdlms/tile.hpp"

Tile::Tile(Sprite sprite, int layer, int z, int id) : _sprite(sprite),
                                                      _layer(layer),
                                                      _z(z)
{
}
void Tile::draw()
{
    _sprite.draw();
}
