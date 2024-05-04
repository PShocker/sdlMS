#include "PlayerSystem.h"
#include "Core/Input.h"
#include "Components/Avatar.h"

void PlayerSystem::run(World &world)
{
    if (world.components_exist_of_type<Player>())
    {
        for (auto &[index, pla] : world.get_components<Player>())
        {
            update_pla(pla, world);
        }
    }
}

void PlayerSystem::update_pla(Player *pla, World &world)
{
    auto nor = pla->get_owner_component<Normal>();
    auto ava = pla->get_owner_component<Avatar>();

    switch (nor->type)
    {
    case Normal::GROUND:
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
        if (Input::is_key_held(SDLK_LALT))
        {
            nor->type = Normal::AIR;
            nor->vspeed = -555;
            ava->switch_act(Avatar::ACTION::JUMP);
        }
        break;
    case Normal::AIR:
        if (Input::is_key_held(SDLK_RIGHT))
        {
            ava->direct = false;
        }
        else if (Input::is_key_held(SDLK_LEFT))
        {
            ava->direct = true;
        }
        break;
    case Normal::CLIMB:

        break;
    }
    // int x1 = Input::is_key_held(SDLK_RIGHT) || Input::is_key_held(SDLK_d); // ? 1 : 0;
}
