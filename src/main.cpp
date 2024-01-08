#include <SDL2/SDL.h>
#include <stdio.h>

#include <wz/Node.hpp>
#include <wz/File.hpp>
#include <wz/Directory.hpp>
#include <wz/Property.hpp>

#include "util/string_util.hpp"
#include "util/wz_util.hpp"
#include "worldmap/worldmap.hpp"

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;

SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

int main(int argc, char *args[])
{
    WzUtil::init();
    int mapId = 100000000;

    // The window we'll be rendering to
    SDL_Window *window = NULL;
    // The surface contained by the window
    SDL_Surface *screenSurface = NULL;
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        // Create window
        window = SDL_CreateWindow("sdlMS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
            auto world_map = worldmap::WorldMap(mapId, renderer);

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
                // 处理输入
                const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
                if (keyboardState[SDL_SCANCODE_LEFT])
                {
                    camera.x -= 5;
                    // break;
                }
                if (keyboardState[SDL_SCANCODE_RIGHT])
                {
                    camera.x += 5;
                }
                if (keyboardState[SDL_SCANCODE_UP])
                {
                    camera.y -= 5;
                }
                if (keyboardState[SDL_SCANCODE_DOWN])
                {
                    camera.y += 5;
                }

                // 更新屏幕
                auto tile = world_map.get_tile();
                for (auto it : tile)
                {
                    SDL_RenderCopy(renderer, it.get_texture(), NULL, it.get_rect());
                }

                // auto tile=worldmap::WorldMap::current()->get_tile();
                SDL_RenderPresent(renderer);
                SDL_Delay(16);
                SDL_RenderClear(renderer);
            }
        }
    }
    // Destroy window
    SDL_DestroyWindow(window);
    // Quit SDL subsystems
    SDL_Quit();
    return 0;
}