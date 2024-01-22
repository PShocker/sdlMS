#ifndef HEADER_SDLMS_MINIMAP
#define HEADER_SDLMS_MINIMAP

#include <SDL2/SDL.h>
#include <map>
#include <string>
#include "sdlms/sprite.hpp"
#include "util/currenton.hpp"

class MiniMap : public Currenton<MiniMap>
{
public:
    void draw();

public:
    std::map<std::string, Sprite> _s;
};

#endif
