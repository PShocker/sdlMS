#include "../util/currenton.hpp"
#include "../sdlms/tile.hpp"

namespace worldmap
{

    class WorldMap final : public Currenton<WorldMap>
    {
    private:
        std::vector<Tile> tile;

    private:
        WorldMap(const WorldMap &) = delete;
        WorldMap &operator=(const WorldMap &) = delete;
    };
}