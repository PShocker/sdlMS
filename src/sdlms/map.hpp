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
#include "sdlms/minimap.hpp"
#include "sdlms/foothold.hpp"
#include "sdlms/ladderRope.hpp"
#include "sdlms/character.hpp"
#include "sdlms/npc.hpp"

class Map : public Currenton<Map>
{
public:
    Map();
    void draw();
    void update(int elapsedTime);
    void init(int mapId);

public:
    std::tuple<std::vector<uint8_t>, int> _sound; // pcm流,采样率

    std::tuple<int, int, int, int> _border; // 摄像机范围

    std::u16string bgm;
};

#endif