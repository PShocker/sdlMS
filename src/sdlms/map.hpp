#ifndef HEADER_SDLMS_MAP
#define HEADER_SDLMS_MAP
#include <vector>
#include <SDL2/SDL.h>

#include "util/currenton.hpp"
#include "sdlms/tile.hpp"
#include  "sdlms/graphics.hpp"

class Map : public Currenton<Map>
{
public:
    void draw();

public:
    std::vector<Tile> _tile;

};

#endif