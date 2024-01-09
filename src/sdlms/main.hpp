#ifndef HEADER_SDLMS_MAIN
#define HEADER_SDLMS_MAIN

#include "worldmap/camera.hpp"
#include "util/map_util.hpp"
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
    std::unique_ptr<WzUtil> _wz_util;
    std::unique_ptr<MapUtil> _map_util;
    std::unique_ptr<Camera> _camera;
    std::unique_ptr<Graphics> _graphics;
    std::unique_ptr<Input> _input;
};
#endif