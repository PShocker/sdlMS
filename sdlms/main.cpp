#include <process.h>

import systems;
import core;
import resources;

int width = 100;
int height = 100;

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

    Render::run();
    Animate::run();

    Window::update();
}

int main(int argc, char *argv[])
{
    Camera::y = -500;
    
    Camera::w = width;
    Camera::h = height;

    Wz::init("./Data/");
    Window::create_window("sdlMS", width, height);

    Map::load(100000000);

    while (true)
    {
        main_loop();
    }
}