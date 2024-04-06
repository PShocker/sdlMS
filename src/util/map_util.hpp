#ifndef HEADER_UTIL_MAP
#define HEADER_UTIL_MAP

#include <vector>
#include <ranges>
#include "wz/Property.hpp"

#include "template/currenton.hpp"
#include "util/sprite_util.hpp"

#include "sdlms/tile.hpp"
#include "sdlms/obj.hpp"
#include "sdlms/backgrd.hpp"
#include "sdlms/portal.hpp"
#include "sdlms/foothold.hpp"
#include "sdlms/rope.hpp"

// 地图加载的类,用来加载地图上的资源
namespace util
{
    class MapUtil final : public Currenton<MapUtil>
    {
    public:
        MapUtil();

    public:
        std::array<std::vector<Tile>, 8> load_tile(int mapId);
        std::array<std::vector<Obj>, 8> load_obj(int mapId);
        std::vector<BackGrd> load_backgrd(int mapId);
        std::vector<Portal> load_portal(int mapId);
        std::optional<Sprite> load_minimap(int mapId);
        std::optional<Sprite> load_mark(int mapId);
        std::unordered_map<int, FootHold> load_foothold(int mapId);
        std::unordered_map<int, Rope> load_rope(int mapId);
        wz::Node *load_node(int mapId);

    private:
        SpriteUtil *_sprite_util;

    private:
        wz::Node *_map_node;

    private:
        std::vector<Tile> load_tile(wz::Node *node, int i);
        std::vector<Obj> load_obj(wz::Node *node, int i);
        std::vector<BackGrd> load_backgrd(wz::Node *node);
        std::vector<Portal> load_portal(wz::Node *node);
        std::unordered_map<int, FootHold> load_foothold(wz::Node *node);
        std::unordered_map<int, Rope> load_rope(wz::Node *node);
    };
}

#endif