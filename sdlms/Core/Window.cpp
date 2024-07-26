module;

#include <cstdlib>
#include <SDL3/SDL.h>

module core;

void Window::create_window(const char *title, unsigned int width, unsigned int height)
{
    int result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    if (result < 0)
    {
        std::abort();
    }
    window = SDL_CreateWindow(title, width, height, 0);
    if (!window)
    {
        std::abort();
    }
    renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderLogicalPresentation(renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL_SCALEMODE_NEAREST);
    window_title = title;
}

void Window::destroy_window()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void Window::poll_events()
{
    SDL_Event event_handler;

    while (SDL_PollEvent(&event_handler) != 0)
    {
        if (event_handler.type == SDL_EVENT_QUIT)
        {
            quit = true;
            break;
        }
    }
}

void Window::tick_delta_time()
{
    dt_last = dt_now;
    dt_now = SDL_GetTicks();
    delta_time = dt_now - dt_last;
}

void Window::clear()
{
    SDL_RenderClear(renderer);
}

void Window::update()
{
    SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_RenderPresent(renderer);
}
