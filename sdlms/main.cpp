#include <process.h>

import systems;
import core;
import resources;
import components;
import entities;
import ui;

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

    animate_run();
    statemachine_run();
    camera_run();
    world_run();
    render_run();

    Window::update();
}

int main(int argc, char *argv[])
{
    Camera::x = 0;
    Camera::y = 0;

    Camera::w = width;
    Camera::h = height;

    Wz::init("./Data/");
    Window::create_window("sdlMS", width, height);
    Character::init();
    AfterImage::init();
    Cursor::init();

    Map::load(105040300);
    
    load_character(0, 0);

    Window::tick_delta_time();
    while (true)
    {
        main_loop();
    }
}