#include <SDL2/SDL.h>
#include <stdio.h>
#include "Core/World.h"
#include "Systems/RenderSystem.h"
#include "Systems/SoundSystem.h"
#include "Systems/UpdateSystem.h"
#include "Systems/CameraSystem.h"
#include "Systems/PhysicSystem.h"
#include "Systems/PlayerSystem.h"
#include "Systems/SpriteSystem.h"
#include "Systems/TransformSystem.h"
#include "Components/Sound.h"
#include "Components/Camera.h"
#include "Components/Player.h"
#include "Components/Physic/Normal.h"
#include "Components/Video.h"
#include "Entities/Character.h"
#include "Entities/Mob.h"
#include "Core/Map.h"
#include "Core/FreeType.h"
#include "Resource/Wz.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __ANDROID__
#include "SDL_main.h"
#endif

int width = 800;
int height = 600;

void main_loop()
{
    World *world = World::get_world();
    [[unlikely]]
    if (world->is_game_quit())
    {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop(); /* this should "kill" the app. */
#else
        exit(0);
#endif
    }
    world->poll_events();
    world->tick_delta_time();

    Window::clear();

    world->process_systems();
    Window::update();
}

int main(int argc, char *argv[])
{
    World world;
#ifdef __ANDROID__
    Wz *wz = new Wz(""); // wz文件路径
#else
    Wz *wz = new Wz("Data/"); // wz文件路径
#endif
    world.add_resource(wz);

    Window::create_window("sdlMS", width, height);

    FreeType::init();

    world.add_system(new SoundSystem());

    world.add_system(new PlayerSystem());

    world.add_system(new PhysicSystem());

    world.add_system(new CameraSystem());

    world.add_system(new UpdateSystem());

    world.add_system(new SpriteSystem());

    world.add_system(new TransformSystem());

    world.add_system(new RenderSystem());

    Character *cha = new Character(&world);
    Transform *t = new Transform{2500, 1700};
    Camera *camera = new Camera(0, 0, width, height);
    Normal *nor = new Normal();
    Player *pla = new Player(); // 玩家控制的角色
    cha->add_component(t);
    cha->add_component(camera);
    cha->add_component(nor);
    cha->add_component(pla);
    world.add_component(t, 3000000);
    world.add_component(camera, 0);
    world.add_component(nor);
    world.add_component(pla);
    // Mob *mob = new Mob(&world);
    // mob->add_component(t);
    // mob->add_component(nor);
    // mob->add_component(pla);
    // mob->add_component(camera);
    // world.add_component(t, 3000000);
    // world.add_component(camera, 0);
    // world.add_component(nor);
    // world.add_component(pla);

    Map::load(102000000, &world);

    // Video *vid = Video::load("a.mp4", 244, 184);
    // if (vid != nullptr)
    // {
    //     Entity *ent = new Entity();
    //     Transform *t = new Transform{(float)244, (float)184};
    //     ent->add_component(vid);
    //     ent->add_component(t);
    //     world.add_component(t,123000);
    //     world.add_component(vid);
    // }
    world.tick_delta_time();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (true)
    {
        main_loop();
    }
#endif
    return 0;
}