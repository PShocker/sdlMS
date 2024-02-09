#include "sdlms/main.hpp"
#include <SDL2/SDL.h>

const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

int Main::run(int argc, char **argv)
{
    int mapId = 100000000;
    // int mapId = 222020111;

    _graphics.reset(new Graphics());

    _ffmpeg.reset(new FFMPEG());

    _wz_util.reset(new WzUtil());
    _sprite_util.reset(new SpriteUtil());
    _map_util.reset(new MapUtil());
    _sound_util.reset(new SoundUtil());
    _string_util.reset(new StringUtil());
    _freetype.reset(new FreeType());
    _hud_util.reset(new HudUtil());
    _cursor_util.reset(new CursorUtil());
    _character_util.reset(new CharacterUtil());

    _input.reset(new Input());
    _map.reset(new Map());
    _hud.reset(new Hud());
    _camera.reset(new Camera());
    _cursor.reset(new Cursor());

    _cursor->_s = _cursor_util->load();

    _sound.reset(new Sound());

    _hud->_minimap->_s = _hud_util->load_minimap(mapId);
    _hud->_statusbar->_s = _hud_util->load_statusbar();
    _hud->_statusbar->_e = _hud_util->load_event_sprite();

    _map->_tile = _map_util->load_tile(mapId);
    _map->_obj = _map_util->load_obj(mapId);
    _map->_backgrd = _map_util->load_backgrd(mapId);
    _map->_portal = _map_util->load_portal(mapId);

    _map->_sound = _ffmpeg->decodeAudioToPCM(_sound_util->load_sound(mapId));

    _sound->load(_map->_sound);
    _cursor->load();

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
        // 更新屏幕
        Graphics::current()->clear();
        _map->draw();
        _hud->draw();
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