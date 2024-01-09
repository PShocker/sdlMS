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
    SDL_Renderer *_renderer=_graphics->getRenderer();

    _wzUtil.reset(new WzUtil());
    _worldMap.reset(new WorldMap(mapId, _renderer));

    SDL_Event event;
    while (true)
    {
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                break;
            }
        }
        // 更新屏幕
        for (auto it : _worldMap->get_tile())
        {
            SDL_RenderCopy(_renderer, it.get_texture(), NULL, it.get_rect());
        }

        // auto tile=worldmap::WorldMap::current()->get_tile();
        SDL_RenderPresent(_renderer);
        SDL_Delay(16);
        SDL_RenderClear(_renderer);
    }

    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 0;
}