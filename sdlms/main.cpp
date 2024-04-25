#include <SDL2/SDL.h>
#include <stdio.h>
#include "Core/World.h"

int main(int argc, char *argv[])
{
    Window::create_window("SDL Test", 640, 480);

    World world;

    while (!world.is_game_quit())
    {
        world.poll_events();
        world.tick_delta_time();

        Window::clear();

        world.process_systems();

        Window::update();
    }
    return 0;
}