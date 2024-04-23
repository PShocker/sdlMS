#ifndef HEADER_UTIL_MAP
#define HEADER_UTIL_MAP

#include <vector>
#include <ranges>
#include "wz/Property.hpp"

#include "template/currenton.hpp"
#include "util/string_util.hpp"
#include "util/freetype_util.hpp"


#include "sdlms/tile.hpp"
#include "sdlms/obj.hpp"
#include "sdlms/backgrd.hpp"
#include "sdlms/portal.hpp"
#include "sdlms/foothold.hpp"
#include "sdlms/ladderRope.hpp"
#include "sdlms/npc.hpp"

// 地图加载的类,用来加载地图上的资源
namespace util
{
    class MapUtil final : public Currenton<MapUtil>
    {
    public:
        MapUtil();

    public:
        std::tuple<int, int, int, int> load_border(int mapId);
        wz::Node *load_map_node(int mapId);

    private:
        std::tuple<int, int, int, int> load_border(wz::Node *node);
    };
}

#endif