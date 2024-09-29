#include "entt/entt.hpp"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

import systems;
import core;
import resources;
import components;
import entities;
import ui;

int width = 800;
int height = 600;

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Window::tick_delta_time();
    Window::clear();
    cooldown_run();
    animate_run();
    player_statemachine_run();
    attack_run();
    mob_statemachine_run();
    world_run();
    camera_run();
    render_run();
    Window::update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    Camera::w = width;
    Camera::h = height;

    Wz::init("./Data/");
    Window::create_window("sdlMS", width, height);
    Character::init();
    AfterImage::init();
    Tomb::init();
    Damage::init();
    Cursor::init();
    Sound::init();

    World::load_map(101000000);

    Player::ent = World::registry->create();
    load_character(0, 0, true, &Player::ent);

    auto e = World::registry->create();
    load_character(-450, 260, false, &e);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    return (event->type == SDL_EVENT_QUIT) ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate)
{
    SDL_Quit();
}