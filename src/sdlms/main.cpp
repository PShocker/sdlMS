#include "sdlms/main.hpp"
#include <SDL2/SDL.h>

const int FPS = 100;
const int FRAME_DELAY = 1000 / FPS;

int Main::run(int argc, char **argv)
{
    int mapId = 103020200;
    // int mapId = 222020111;

    _graphics.reset(new Graphics());

    _ffmpeg_util.reset(new FfmpegUtil());

    _wz_util.reset(new WzUtil());
    _sprite_util.reset(new SpriteUtil());
    _map_util.reset(new MapUtil());
    _sound_util.reset(new SoundUtil());
    _string_util.reset(new StringUtil());
    _freetype_util.reset(new FreeTypeUtil());
    _cursor_util.reset(new CursorUtil());
    _character_util.reset(new CharacterUtil());
    _statusbar_util.reset(new StatusBarUtil());
    _minimap_util.reset(new MiniMapUtil());

    _input.reset(new Input());
    _map.reset(new Map());
    _character.reset(new Character());
    _camera.reset(new Camera());
    _cursor.reset(new Cursor());
    _minimap.reset(new MiniMap());
    _statusbar.reset(new StatusBar());

    _cursor->_s = _cursor_util->load();

    _sound.reset(new Sound());

    _minimap->_s = _minimap_util->load_minimap(mapId);
    _statusbar->_s = _statusbar_util->load_statusbar();
    _statusbar->_e = _statusbar_util->load_event_sprite();

    _map->_tile = _map_util->load_tile(mapId);
    _map->_obj = _map_util->load_obj(mapId);
    _map->_backgrd = _map_util->load_backgrd(mapId);
    _map->_portal = _map_util->load_portal(mapId);
    _map->_foothold = _map_util->load_foothold(mapId);

    _map->_sound = _ffmpeg_util->decodeAudioToPCM(_sound_util->load_sound(mapId));

    _sound->load(_map->_sound);
    _cursor->load();

    _character->_s=_character_util->load();

    unsigned int frameStart;
    unsigned int frameTime;

    while (true)
    {
        frameStart = SDL_GetTicks(); // 获取当前帧开始的时间
        if (_input->loop() < 0)
        {
            break;
        } 
        _map->update(frameStart - frameTime);
        _character->update(frameStart - frameTime);
        // 更新屏幕
        Graphics::current()->clear();
        _map->draw();
        _minimap->draw();
        _statusbar->draw();
        _character->draw();
        Graphics::current()->flip();
        frameTime = SDL_GetTicks();
        if (frameTime - frameStart < FRAME_DELAY)
        {
            SDL_Delay(FRAME_DELAY - (frameTime - frameStart)); // 控制帧率
        }
    }

    printf("SDL_Error: %s\n", SDL_GetError());
    return 0;
}