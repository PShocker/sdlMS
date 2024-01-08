#include "sdlms/main.hpp"
#include <SDL2/SDL.h>
#include "main.hpp"

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;

Main::Main():m_wz_util()
{
    printf("123");
}

int Main::run(int argc, char **argv)
{
    int mapId = 100000000;

    // The window we'll be rendering to
    SDL_Window *window = NULL;
    // The surface contained by the window
    SDL_Surface *screenSurface = NULL;
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        return 0;
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
            // m_world_map = worldmap::WorldMap(mapId, renderer);
        }
    }
    return 0;
}