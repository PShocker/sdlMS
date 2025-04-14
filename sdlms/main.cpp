#include "Systems/Systems.h"
#include "Core/Core.h"
#include "Entities/Entities.h"
#include "UI/UI.h"
#include "Resources/Wz.h"
#include "entt/entt.hpp"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int width = 1366;
int height = 768;

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Window::tick_delta_time();
    Window::clear();
    animate_run();
    player_statemachine_run();
    mob_statemachine_run();
    npc_statemachine_run();
    buff_run();
    drop_run();
    ball_run();
    ui_run();
    world_run();
    summon_statemachine_run();
    pet_statemachine_run();
    camera_run();
    render_run();
    Window::update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (argc == 3)
    {
        width = SDL_atoi(argv[1]);
        height = SDL_atoi(argv[2]);
    }

    Camera::w = width;
    Camera::h = height;

    FreeType::init("./Font/");
    Wz::init("./Data/");
    Keyboard::init();
    Window::create_window("sdlMS", width, height);
    Character::init();
    Tomb::init();
    Damage::init();
    Cursor::init();
    StatusBar::init();
    QuickSlot::init();
    MiniMap::init();
    GainTip::init();
    UIBuff::init();
    Sound::init();
    Drop::init();

    // World::load_map(222020111);

    World::load_map(1000000);
    Player::ent = World::registry->create();
    load_character(0, 0, true, Player::name, Player::ent);
    load_pet(u"5000016", Player::ent);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    return (event->type == SDL_EVENT_QUIT) ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_Quit();
}