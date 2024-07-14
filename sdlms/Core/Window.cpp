#include "Window.h"
#include "Input.h"
#include <cstdlib>

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

    Input::update_momentary_keys();

    while (SDL_PollEvent(&event_handler) != 0)
    {
        switch (event_handler.type)
        {
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            Input::process_input_event(event_handler.type, event_handler);
        }
        break;
        case SDL_EVENT_QUIT:
        {
            quit = true;
        }
        break;
        }
    }
}

void Window::tick_delta_time()
{
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
