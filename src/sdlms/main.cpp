#include "sdlms/main.hpp"
#include <SDL2/SDL.h>

const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

int Main::run(int argc, char **argv)
{
    int mapId = 100000000;
    // int mapId = 222020110;

    _graphics.reset(new Graphics());

    _ffmpeg.reset(new FFMPEG());

    _wz_util.reset(new WzUtil());
    _map_util.reset(new MapUtil());
    _sound_util.reset(new SoundUtil());
    _hud_util.reset(new HudUtil());

    _map.reset(new Map());
    _hud.reset(new Hud());
    _camera.reset(new Camera());
    _input.reset(new Input());

    _sound.reset(new Sound());

    _freetype.reset(new FreeType());

    _hud->_minimap->_s = _hud_util->load_minimap(mapId);

    _map->_tile = _map_util->load_tile(mapId);
    _map->_obj = _map_util->load_obj(mapId);
    _map->_backgrd = _map_util->load_backgrd(mapId);
    _map->_portal = _map_util->load_portal(mapId);

    _map->_sound = _ffmpeg->decodeAudioToPCM(_sound_util->load_sound(mapId));

    _sound->load(_map->_sound);

    while (true)
    {
        auto frameStart = SDL_GetTicks(); // 获取当前帧开始的时间

        if (_input->loop() < 0)
        {
            break;
        }
        _map->update(FRAME_DELAY);
        // 更新屏幕
        Graphics::current()->clear();
        _map->draw();
        _hud->draw();
        Graphics::current()->flip();
        auto frameTime = SDL_GetTicks() - frameStart; // 计算当前帧的时间
        if (frameTime < FRAME_DELAY)
        {
            SDL_Delay(FRAME_DELAY - frameTime); // 控制帧率
        }
    }

    printf("SDL_Error: %s\n", SDL_GetError());
    return 0;
}