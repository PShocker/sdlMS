#ifndef HEADER_WORLDMAP_WORLDMAP
#define HEADER_WORLDMAP_WORLDMAP

#include <vector>
#include "wz/Property.hpp"

#include "util/currenton.hpp"
#include "sdlms/tile.hpp"
#include "sdlms/obj.hpp"

namespace util
{

    class MapUtil final : public Currenton<MapUtil>
    {
    public:
        MapUtil();

    public:
        std::vector<Tile> load_tile(int mapId);
        std::vector<Obj> load_obj(int mapId);

    private:
        SDL_Renderer *_renderer;

    private:
        void load_tile(wz::Node *root, wz::Node *node, int i, std::vector<Tile> &tile);
        void load_obj(wz::Node *root, wz::Node *node, int i, std::vector<Obj> &obj);
    };
}

#endif