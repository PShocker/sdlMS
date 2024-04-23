#ifndef HEADER_SDLMS_MAIN
#define HEADER_SDLMS_MAIN


#include "util/map_util.hpp"
#include "util/wz_util.hpp"
#include "util/sound_util.hpp"
#include "util/ffmpeg_util.hpp"
#include "util/freetype_util.hpp"
#include "util/string_util.hpp"
#include "util/cursor_util.hpp"
#include "util/character_util.hpp"
#include "util/minimap_util.hpp"

#include "sdlms/camera.hpp"
#include "sdlms/graphics.hpp"
#include "sdlms/input.hpp"
#include "sdlms/map.hpp"
#include "sdlms/sound.hpp"
#include "sdlms/cursor.hpp"
#include "sdlms/minimap.hpp"
#include "sdlms/statusbar.hpp"
#include "sdlms/physics.hpp"

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
    std::unique_ptr<MiniMap> _minimap;
    std::unique_ptr<StatusBar> _statusbar;
    std::unique_ptr<Sound> _sound;
    std::unique_ptr<Physics> _physics;
    std::unique_ptr<FfmpegUtil> _ffmpeg_util;
    std::unique_ptr<FreeTypeUtil> _freetype_util;
    std::unique_ptr<StringUtil> _string_util;
    std::unique_ptr<CursorUtil> _cursor_util;
    std::unique_ptr<Cursor> _cursor;
    std::unique_ptr<Character> _character;
    std::unique_ptr<CharacterUtil> _character_util;
    std::unique_ptr<MiniMapUtil> _minimap_util;
    
};
#endif