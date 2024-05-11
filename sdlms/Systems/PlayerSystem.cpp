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
    nor->want_prone = false;
    nor->want_fall = false;
    nor->want_stand = false;
    nor->want_jump = false;

    switch (nor->type)
    {
    case Normal::Ground:
        if (Input::is_key_held(SDLK_RIGHT)) // flip(1)
        {
            nor->hkey = Normal::Right;
            // nor->hforce = 1400;
            // tr->set_flip(1);
            // if (ava != nullptr)
            // {
            //     ava->switch_act(Avatar::ACTION::WALK1);
            // }
        }
        else if (Input::is_key_held(SDLK_LEFT)) // flip(0)
        {
            nor->hkey = Normal::Left;
            // nor->hforce = -1400;
            // tr->set_flip(0);
            // if (ava != nullptr)
            // {
            //     ava->switch_act(Avatar::ACTION::WALK1);
            // }
        }
        else
        {
            nor->want_stand = true;
        }
        if (Input::is_key_held(SDLK_UP))
        {
            nor->vkey = Normal::Up;
        }
        else if (Input::is_key_held(SDLK_DOWN))
        {
            nor->vkey = Normal::Down;
            nor->want_prone = true;
        }
        if (Input::is_key_held(SDLK_LALT) && Input::is_key_held(SDLK_DOWN))
        {
            nor->want_fall = true;
        }
        else if (Input::is_key_held(SDLK_LALT))
        {
            nor->want_jump = true;
        }
        break;
    case Normal::Air:
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
        break;
    case Normal::Climb:
        if (Input::is_key_held(SDLK_RIGHT))
        {
            // tr->set_flip(1);
            // nor->hspeed = 100;
            nor->hkey = Normal::Right;
        }
        else if (Input::is_key_held(SDLK_LEFT))
        {
            // tr->set_flip(0);
            // nor->hspeed = -100;
            nor->hkey = Normal::Left;
        }
        if (Input::is_key_held(SDLK_LALT) && !Input::is_key_held(SDLK_UP) && (Input::is_key_held(SDLK_LEFT) || Input::is_key_held(SDLK_RIGHT)))
        {
            nor->want_jump = true;
            break;
        }
        if (Input::is_key_held(SDLK_UP))
        {
            nor->vkey = Normal::Up;
        }
        else if (Input::is_key_held(SDLK_DOWN))
        {
            nor->vkey = Normal::Down;
        }
        break;
    }
}
