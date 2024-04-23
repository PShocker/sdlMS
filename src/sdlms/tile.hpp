#ifndef HEADER_SDLMS_TILE
#define HEADER_SDLMS_TILE

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/sprite.hpp"

class Tile : public Sprite
{
public:
    Tile(Sprite sprite, int layer, int z);
    std::array<std::vector<Tile>, 8> load_tile(int mapId);
    std::vector<Tile> load_tile(wz::Node *node, int i);

    static void draws(std::vector<Tile> &tiles);

public:
    int _layer;
    int _z;
};
#endif