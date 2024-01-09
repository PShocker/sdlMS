#ifndef HEADER_SDLMS_MAIN
#define HEADER_SDLMS_MAIN

#include "worldmap/camera.hpp"
#include "worldmap/worldmap.hpp"
#include "util/wz_util.hpp"
#include "sdlms/graphics.hpp"
#include "sdlms/input.hpp"

using namespace worldmap;
using namespace util;

class Main final
{

public:
    int run(int argc, char **argv);

private:
    std::unique_ptr<WzUtil> _wzUtil;
    std::unique_ptr<WorldMap> _worldMap;
    std::unique_ptr<Camera> _camera;
    std::unique_ptr<Graphics> _graphics;
    std::unique_ptr<Input> _input;
};
#endif