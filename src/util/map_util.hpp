#ifndef HEADER_WORLDMAP_WORLDMAP
#define HEADER_WORLDMAP_WORLDMAP

#include <vector>
#include "wz/Property.hpp"

#include "util/currenton.hpp"
#include "sdlms/tile.hpp"
#include "sdlms/obj.hpp"
#include "sdlms/backgrd.hpp"

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

    private:
        SDL_Renderer *_renderer;

    private:
        std::vector<Tile> load_tile(wz::Node *root, wz::Node *node, int i);
        std::vector<Obj> load_obj(wz::Node *root, wz::Node *node, int i);
        std::vector<BackGrd> load_backgrd(wz::Node *root, wz::Node *node);
    };
}

#endif