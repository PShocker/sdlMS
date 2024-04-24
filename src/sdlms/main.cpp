#include "sdlms/main.hpp"
#include <SDL2/SDL.h>

const unsigned int FPS = 60;
const unsigned int FRAME_DELAY = 1000 / FPS;

int Main::run(int argc, char **argv)
{
    int mapId = 10000;
    // int mapId = 104000000;
    // int mapId = 105040300;
    // int mapId = 104040000;
    // int mapId = 222020111;

    _graphics.reset(new Graphics());

    _ffmpeg_util.reset(new FfmpegUtil());

    _wz_util.reset(new WzUtil());
    _sound_util.reset(new SoundUtil());
    _string_util.reset(new StringUtil());
    _freetype_util.reset(new FreeTypeUtil());
    _map_util.reset(new MapUtil());
    _cursor_util.reset(new CursorUtil());
    _character_util.reset(new CharacterUtil());

    _input.reset(new Input());
    _map.reset(new Map());
    _character.reset(new Character());
    _camera.reset(new Camera());
    _cursor.reset(new Cursor());
    _minimap.reset(new MiniMap(mapId));
    _statusbar.reset(new StatusBar());

    _cursor->_s = _cursor_util->load();

    _sound.reset(new Sound());

    _map->init(mapId);

    _physics.reset(new Physics());

    _map->_sound = _ffmpeg_util->decodeAudioToPCM(_sound_util->load_sound(mapId));

    // _sound->load(_map->_sound);
    _cursor->load();

    _character->_s = _character_util->load();

    unsigned int frameStart, frameTime;
    frameStart = frameTime = SDL_GetTicks();

    while (true)
    {
        frameStart = SDL_GetTicks(); // 获取当前帧开始的时间
        auto elapsedTime = std::max(frameStart - frameTime, FRAME_DELAY);

        if (_input->loop() < 0)
        {
            break;
        }
        _map->update(elapsedTime);
        _physics->update(elapsedTime);
        _character->update(elapsedTime);
        _camera->update(elapsedTime);

        // 更新屏幕
        Graphics::current()->clear();
        _map->draw();
        _minimap->draw();
        _statusbar->draw();
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