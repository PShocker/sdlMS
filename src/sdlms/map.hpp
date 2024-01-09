#ifndef HEADER_SDLMS_MAIN
#define HEADER_SDLMS_MAIN
#include "util/currenton.hpp"

class Map : public Currenton<Map>
{
public:
    std::vector<Tile> _tile;
}

#endif