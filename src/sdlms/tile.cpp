#include "sdlms/tile.hpp"

Tile::Tile(Sprite sprite, int layer, int z) : Sprite(sprite),
                                              _layer(layer),
                                              _z(z)
{
}

void Tile::draws(std::vector<Tile> &tiles)
{
    for (auto &it : tiles)
    {
        it.draw();
    }
}
