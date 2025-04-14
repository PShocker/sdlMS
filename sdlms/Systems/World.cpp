#include "World.h"
#include "Camera.h"
#include "Systems/PlayerStateMachine.h"
#include "Commons/Commons.h"
#include "Core/Core.h"
#include "Entities/Entities.h"
#include "Components/Components.h"
#include "entt/entt.hpp"

void world_run()
{
    world_destory_ent();
    world_transport();
    world_sort_zindex();
}

void world_sort_zindex()
{
    if (World::zindex)
    {
        World::registry->sort<Transform>([](const auto &lhs, const auto &rhs)
                                         { return lhs.z < rhs.z; });
        World::zindex = false;
    }
}

void world_transport()
{
    if (World::TransPort::id != 0)
    {
        World::load_map(World::TransPort::id);
        if (World::TransPort::tn == u"jumpSP")
        {
            World::TransPort::tn = u"sp";
        }
        for (auto ent : World::registry->view<Portal>())
        {
            auto portal = World::registry->try_get<Portal>(ent);
            if (portal->pn == World::TransPort::tn)
            {
                World::registry->remove<Tomb>(Player::ent);
                auto tr = World::registry->try_get<Transform>(Player::ent);
                auto mv = World::registry->try_get<Move>(Player::ent);
                auto cha = World::registry->try_get<Character>(Player::ent);
                tr->position = World::registry->try_get<Transform>(ent)->position + SDL_FPoint{0, -10};
                tr->z = LAYER_Z * 8 + tr->z % LAYER_Z;
                mv->foo = nullptr;
                mv->vspeed = 0;
                mv->hspeed = 0;
                mv->page = -1;
                cha->state = Character::State::JUMP;
                cha->action = Character::Action::JUMP;
                cha->action_str = u"jump";
                cha->action_frame = 0;
                cha->action_index = 0;
                cha->action_time = 0;
                cha->invincible_cooldown = 0;
                cha->animate = true;
                camera_refresh();
                World::TransPort::id = 0;
                break;
            }
        }
    }
}

void world_destory_ent()
{
    for (auto ent : World::destory)
    {
        if (World::registry->valid(ent))
        {
            World::registry->destroy(ent);
        }
    }
    World::destory.clear();
}
