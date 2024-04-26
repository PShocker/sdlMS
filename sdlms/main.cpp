#include <SDL2/SDL.h>
#include <stdio.h>
#include "Core/World.h"
#include "Entities/Tile.h"
#include "Systems/RenderSystem.h"
#include "Systems/SoundSystem.h"
#include "Components/Sound.h"
#include "Core/Scene.h"
#include "Resource/Wz.h"

int main(int argc, char *argv[])
{
    World world;
    Wz *wz = new Wz("./data/"); // wz文件路径
    world.add_resource(typeid(Wz), wz);

    Window::create_window("sdlMS", 640, 480);

    Scene *scene = new Scene(&world);
    scene->load_map(10000);

    RenderSystem rsys{};
    world.add_system(&rsys);

    Sound *sou = Sound::load_sound(nullptr);

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