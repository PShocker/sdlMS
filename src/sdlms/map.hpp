#ifndef HEADER_SDLMS_MAP
#define HEADER_SDLMS_MAP
#include <vector>
#include <array>
#include <SDL2/SDL.h>

#include "util/currenton.hpp"
#include "sdlms/tile.hpp"
#include "sdlms/obj.hpp"
#include "sdlms/graphics.hpp"

class Map : public Currenton<Map>
{
public:
    void draw();

public:
    std::array<std::vector<Tile>,8> _tile;
    std::vector<Obj> _obj;
};

#endif