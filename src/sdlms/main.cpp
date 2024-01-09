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
    SDL_Renderer *_renderer = _graphics->getRenderer();

    _input.reset(new Input());
    _wzUtil.reset(new WzUtil());
    _mapUtil.reset(new MapUtil());

    auto tiles=_mapUtil->load_tile(mapId,_renderer);

    SDL_Event event;
    while (true)
    {
        _input->beginNewFrame();
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.repeat == 0)
                {
                    _input->keyDownEvent(event);
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                _input->keyUpEvent(event);
            }
            else if (event.type == SDL_QUIT)
            {
                return 0;
            }
        }
        if (_input->wasKeyPressed(SDL_SCANCODE_ESCAPE) == true)
        {
            return 0;
        }
        else if (_input->isKeyHeld(SDL_SCANCODE_LEFT) == true)
        {
            printf("123");
        }
        else if (_input->isKeyHeld(SDL_SCANCODE_RIGHT) == true)
        {
            printf("456");
        }

        // 更新屏幕
        for (auto it : tiles)
        {
            SDL_RenderCopy(_renderer, it.get_texture(), NULL, it.get_rect());
        }

        SDL_RenderPresent(_renderer);
        SDL_Delay(16);
        SDL_RenderClear(_renderer);
    }

    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 0;
}