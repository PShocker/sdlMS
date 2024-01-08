#include "worldmap/camera.hpp"
#include "worldmap/worldmap.hpp"

class Main final
{
public:
    Main();
    int run(int argc, char **argv);

private:
    worldmap::WorldMap m_world_map;
    worldmap::Camera m_camera;
};