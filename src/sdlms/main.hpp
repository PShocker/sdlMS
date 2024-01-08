#include "worldmap/camera.hpp"
#include "worldmap/worldmap.hpp"

class Main final
{
public:
    Main();

private:
    worldmap::WorldMap m_world_map;
    worldmap::Camera m_camera;
};