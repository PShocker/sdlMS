#ifndef HEADER_WORLDMAP_WORLDMAP
#define HEADER_WORLDMAP_WORLDMAP

#include <vector>
#include "wz/Property.hpp"

#include "util/currenton.hpp"
#include "sdlms/tile.hpp"

namespace util
{

    class MapUtil final : public Currenton<MapUtil>
    {
    public:
        MapUtil();

    public:
        std::vector<Tile> load_tile(int mapId);

    private:
        SDL_Renderer *_renderer;

    private:
        void load_tile(wz::Node *root, wz::Node *node, int i, std::vector<Tile> &tile);
    };
}

#endif