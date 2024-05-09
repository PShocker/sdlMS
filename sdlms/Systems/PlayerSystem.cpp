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
    nor->want_climb = Normal::None;
    nor->want_prone = false;
    nor->want_fall = false;

    switch (nor->type)
    {
    case Normal::Ground:
        if (Input::is_key_held(SDLK_RIGHT))
        {
            nor->hforce = 1400;
            ava->direct = false;
            ava->switch_act(Avatar::ACTION::WALK1);
        }
        else if (Input::is_key_held(SDLK_LEFT))
        {
            nor->hforce = -1400;
            ava->direct = true;
            ava->switch_act(Avatar::ACTION::WALK1);
        }
        else
        {
            nor->hforce = 0;
            ava->switch_act(Avatar::ACTION::STAND1);
        }
        if (Input::is_key_held(SDLK_UP))
        {
            nor->want_climb = Normal::Up;
        }
        else if (Input::is_key_held(SDLK_DOWN))
        {
            nor->want_climb = Normal::Down;
            nor->want_prone = true;
        }
        if (Input::is_key_held(SDLK_LALT) && Input::is_key_held(SDLK_DOWN))
        {
            nor->want_fall = true;
        }
        else if (Input::is_key_held(SDLK_LALT))
        {
            nor->type = Normal::Air;
            nor->vspeed = -555;
            ava->switch_act(Avatar::ACTION::JUMP);
        }

        break;
    case Normal::Air:
        if (Input::is_key_held(SDLK_RIGHT))
        {
            ava->direct = false;
        }
        else if (Input::is_key_held(SDLK_LEFT))
        {
            ava->direct = true;
        }
        if (Input::is_key_held(SDLK_UP))
        {
            nor->want_climb = Normal::Up;
        }
        else if (Input::is_key_held(SDLK_DOWN))
        {
            nor->want_climb = Normal::Down;
        }
        break;
    case Normal::Climb:
        nor->hspeed = 0;
        nor->vspeed = 0;
        if (Input::is_key_held(SDLK_RIGHT))
        {
            ava->direct = false;
            nor->hspeed = 100;
        }
        else if (Input::is_key_held(SDLK_LEFT))
        {
            ava->direct = true;
            nor->hspeed = -100;
        }
        if (Input::is_key_held(SDLK_LALT) && !Input::is_key_held(SDLK_UP) && nor->hspeed != 0)
        {
            nor->vspeed = -300;
            nor->type = Normal::Air;
            ava->animate = true;
            ava->switch_act(Avatar::ACTION::JUMP);
            break;
        }
        if (Input::is_key_held(SDLK_UP))
        {
            nor->vspeed = -100;
        }
        else if (Input::is_key_held(SDLK_DOWN))
        {
            nor->vspeed = 100;
        }
        break;
    }
}
