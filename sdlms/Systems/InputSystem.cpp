#include "InputSystem.h"
#include "Core/Input.h"
#include "Components/Avatar.h"
#include "SDL_keycode.h"

void InputSystem::run(World &world)
{
    if (world.components_exist_of_type<Player>())
    {
        for (auto &[index, pla] : world.get_components<Player>())
        {
            update_player(pla, world);
        }
    }
    if (world.components_exist_of_type<RandomInput>())
    {
        for (auto &[index, ran] : world.get_components<RandomInput>())
        {
            update_random_input(ran, world);
        }
    }
}

void InputSystem::update_player(Player *pla, World &world)
{
    auto nor = pla->get_owner_component<Normal>();
    auto ava = pla->get_owner_component<Avatar>();
    auto tr = pla->get_owner_component<Transform>();

    nor->vkey = Normal::None;
    nor->hkey = Normal::None;
    nor->lalt = false;
    nor->lctrl = false;

    // 判断是否有h或v方向输入
    if (Input::is_key_held(SDLK_RIGHT))
    {
        nor->hkey = Normal::Right;
    }
    else if (Input::is_key_held(SDLK_LEFT))
    {
        nor->hkey = Normal::Left;
    }
    if (Input::is_key_held(SDLK_UP))
    {
        nor->vkey = Normal::Up;
    }
    else if (Input::is_key_held(SDLK_DOWN))
    {
        nor->vkey = Normal::Down;
    }
    if (Input::is_key_held(SDLK_LALT))
    {
        nor->lalt = true;
    }
    if (Input::is_key_held(SDLK_LCTRL))
    {
        nor->lctrl = true;
    }
}

void InputSystem::update_random_input(RandomInput *ran, World &world)
{
    auto tick = ran->get_tick();
    if (tick > ran->get_count())
    {
        constexpr uint8_t types[] = {Normal::None, Normal::Left, Normal::Right};
        auto num = types[std::rand() % std::size(types)];
        ran->get_owner_component<Normal>()->hkey = num;
        // ran->get_owner_component<Normal>()->hkey = Normal::Left;
        ran->set_tick(0);
    }
    else
    {
        ran->set_tick(ran->get_tick() + 1);
    }
}
