#ifndef HEADER_SDLMS_MINIMAP
#define HEADER_SDLMS_MINIMAP

#include <SDL2/SDL.h>
#include <map>
#include <string>
#include <memory>
#include <optional>

#include "sdlms/frame.hpp"
#include "template/currenton.hpp"

class MiniMap : public Currenton<MiniMap>
{
public:
    MiniMap(){};
    MiniMap(int mapId);
    void draw();
    std::optional<Sprite> load_minimap_mark(int mapId);

public:
    Sprite _canvas;
    Sprite _mapName;
    Sprite _streetName;
    Sprite _mask;
    std::optional<Sprite> _mark;
    Sprite _title;

private:
    Sprite _nw;
    Sprite _ne;
    Sprite _sw;
    Sprite _se;
    vector<Sprite> _n;
    vector<Sprite> _s;
    vector<Sprite> _w;
    vector<Sprite> _e;
};

#endif
