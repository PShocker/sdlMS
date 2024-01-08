#ifndef HEADER_WORLDMAP_WORLDMAP
#define HEADER_WORLDMAP_WORLDMAP

#include <vector>
#include "util/currenton.hpp"
#include "sdlms/tile.hpp"
#include "wz/Property.hpp"

namespace worldmap
{

    class WorldMap final : public Currenton<WorldMap>
    {
    public:
        WorldMap();
        WorldMap(int mapId, SDL_Renderer *renderer);

    public:
        std::vector<Tile> get_tile();

    private:
        void load_tile(wz::Node *root, wz::Node *node, SDL_Renderer *renderer, int i);

    private:
        std::vector<Tile> tile;

    private:
        WorldMap(const WorldMap &) = delete;
        WorldMap &operator=(const WorldMap &) = delete;
    };
}

#endif