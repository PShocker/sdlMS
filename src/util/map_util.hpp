#ifndef HEADER_WORLDMAP_WORLDMAP
#define HEADER_WORLDMAP_WORLDMAP

#include <vector>
#include "util/currenton.hpp"
#include "sdlms/tile.hpp"
#include "wz/Property.hpp"

namespace util
{

    class MapUtil final : public Currenton<MapUtil>
    {
    public:
        MapUtil();

    public:
        std::vector<Tile> load_tile(int mapId, SDL_Renderer *renderer);

    private:
        void load_tile(wz::Node *root, wz::Node *node, SDL_Renderer *renderer, int i, std::vector<Tile> &tile);
    };
}

#endif