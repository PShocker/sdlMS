#include <process.h>
#include "entt/entt.hpp"

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

    cooldown_run();
    animate_run();
    player_statemachine_run();
    mob_statemachine_run();
    world_run();
    camera_run();
    render_run();
    attack_run();


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

    World::load_map(100000000);

    auto ent = load_character();
    World::registry->emplace<Transform>(*ent, recent_portal(0, 0), 99999999);
    World::registry->emplace<Move>(*ent);

    Window::tick_delta_time();
    while (true)
    {
        main_loop();
    }
}