#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "Core/Window.h"
#include "Resource/Wz.h"

int width = 800;
int height = 600;

void main_loop()
{
    if (Window::quit)
    {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop(); /* this should "kill" the app. */
#else
        exit(0);
#endif
    }
    Window::poll_events();
    Window::tick_delta_time();

    Window::clear();
    Window::update();
}

int main(int argc, char *argv[])
{
    Wz::init("./Data/");
    // SDL_CreateTexture()
    Window::create_window("sdlMS", width, height);
    while (true)
    {
        main_loop();
    }
}