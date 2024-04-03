#ifndef HEADER_SDLMS_MAP
#define HEADER_SDLMS_MAP
#include <vector>
#include <array>
#include <SDL2/SDL.h>

#include "template/currenton.hpp"
#include "sdlms/tile.hpp"
#include "sdlms/obj.hpp"
#include "sdlms/backgrd.hpp"
#include "sdlms/portal.hpp"
#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/minimap.hpp"
#include "sdlms/foothold.hpp"

class Map : public Currenton<Map>
{
public:
    Map();
    void draw();
    void update(int elapsedTime);

    void event(const SDL_Event &ev);

public:
    std::array<std::vector<Tile>, 8> _tile;
    std::array<std::vector<Obj>, 8> _obj;
    std::vector<BackGrd> _backgrd;
    std::vector<Portal> _portal;
    std::tuple<std::vector<uint8_t>, int> _sound; // pcm流,采样率
    std::unordered_map<int, FootHold> _foothold;
};

#endif