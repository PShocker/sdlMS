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

    static void draws(int i);
    static void load_tile(int mapId);
    static inline std::array<std::vector<Tile>, 8> _tile;

public:
    int _layer;
    int _z;
};
#endif