#ifndef HEADER_SDLMS_MAIN
#define HEADER_SDLMS_MAIN

#include "worldmap/camera.hpp"
#include "worldmap/worldmap.hpp"
#include "util/wz_util.hpp"

class Main final
{
public:
    Main();

public:
    int run(int argc, char **argv);

private:
    worldmap::WorldMap m_world_map;
    worldmap::Camera m_camera;
    util::WzUtil m_wz_util;
};
#endif