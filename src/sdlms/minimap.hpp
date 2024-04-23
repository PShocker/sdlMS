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
    Frame _frame;
    Sprite _canvas;
    Sprite _mapName;
    Sprite _streetName;
    Sprite _mask;
    std::optional<Sprite> _mark;
    Sprite _title;
};

#endif
