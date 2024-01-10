#include "sdlms/main.hpp"
#include <SDL2/SDL.h>
#include "main.hpp"

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;

int Main::run(int argc, char **argv)
{
    int mapId = 100000000;

    _graphics.reset(new Graphics());

    _wz_util.reset(new WzUtil());
    _map_util.reset(new MapUtil());

    _map.reset(new Map());
    _camera.reset(new Camera());
    _input.reset(new Input());

    _map->_tile = _map_util->load_tile(mapId);

    while (true)
    {
        if (_input->loop() < 0)
        {
            break;
        }
        // 更新屏幕
        _map->draw();
    }

    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 0;
}