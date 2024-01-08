#include <vector>
#include "util/currenton.hpp"
#include "sdlms/tile.hpp"
#include "wz/Property.hpp"

namespace worldmap
{

    class WorldMap final : public Currenton<WorldMap>
    {
    public:
        WorldMap(int mapId, SDL_Renderer *renderer);
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