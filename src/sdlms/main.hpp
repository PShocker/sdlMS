#ifndef HEADER_SDLMS_MAIN
#define HEADER_SDLMS_MAIN


#include "util/map_util.hpp"
#include "util/wz_util.hpp"
#include "util/sound_util.hpp"
#include "util/ffmpeg.hpp"

#include "sdlms/camera.hpp"
#include "sdlms/graphics.hpp"
#include "sdlms/input.hpp"
#include "sdlms/map.hpp"

using namespace util;

class Main final
{

public:
    int run(int argc, char **argv);

private:
    std::unique_ptr<WzUtil> _wz_util;
    std::unique_ptr<MapUtil> _map_util;
    std::unique_ptr<SoundUtil> _sound_util;
    std::unique_ptr<Camera> _camera;
    std::unique_ptr<Graphics> _graphics;
    std::unique_ptr<Input> _input;
    std::unique_ptr<Map> _map;
    std::unique_ptr<FFMPEG> _ffmpeg;
    
};
#endif