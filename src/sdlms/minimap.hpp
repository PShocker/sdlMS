#ifndef HEADER_SDLMS_MINIMAP
#define HEADER_SDLMS_MINIMAP

#include <SDL2/SDL.h>
#include <map>
#include "util/currenton.hpp"

class MiniMap : public Currenton<MiniMap>
{
public:
	MiniMap();
    void draw();
};

#endif
