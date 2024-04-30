#include <SDL2/SDL.h>
#include <stdio.h>
#include "Core/World.h"
#include "Entities/Tile.h"
#include "Systems/RenderSystem.h"
#include "Systems/SoundSystem.h"
#include "Systems/UpdateSystem.h"
#include "Components/Sound.h"
#include "Components/Camera.h"
#include "Core/Map.h"
#include "Resource/Wz.h"

int main(int argc, char *argv[])
{
    World world;
    Wz *wz = new Wz("./Data/"); // wz文件路径
    world.add_resource(typeid(Wz), wz);

    Window::create_window("sdlMS", 1280, 800);

    Map *map = new Map(&world);
    map->load_map(100000000);

    Camera *camera = new Camera(0, 0, 1280, 800);
    world.add_component(camera, 0);

    UpdateSystem upd{};
    world.add_system(&upd);

    RenderSystem rsys{};
    world.add_system(&rsys);

    SoundSystem sous{};
    world.add_system(&sous);

    while (!world.is_game_quit())
    {

        world.poll_events();
        world.tick_delta_time();

        Window::clear();

        world.process_systems();

        Window::update();

        SDL_Delay(20); // 控制帧率
    }
    return 0;
}