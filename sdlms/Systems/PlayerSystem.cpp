#include "PlayerSystem.h"
#include "Core/Input.h"
#include "Components/Avatar.h"

void PlayerSystem::run(World &world)
{
    if (world.components_exist_of_type<Player>())
    {
        for (auto &[index, pla] : world.get_components<Player>())
        {
            update_player(pla, world);
        }
    }
}

void PlayerSystem::update_player(Player *pla, World &world)
{
    auto nor = pla->get_owner_component<Normal>();
    auto ava = pla->get_owner_component<Avatar>();
    auto tr = pla->get_owner_component<Transform>();

    nor->vkey = Normal::None;
    nor->hkey = Normal::None;
    nor->lalt = false;

    //判断是否有h或v方向输入
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
}
