#ifndef HEADER_SDLMS_TILE
#define HEADER_SDLMS_TILE

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/sprite.hpp"

class Tile
{
public:
    Tile(Sprite sprite, int layer, int z);

    void draw();


public:
    Sprite _sprite;
    int _layer;
    int _z;
};
#endif