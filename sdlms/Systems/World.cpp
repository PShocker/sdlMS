#include "World.h"
#include "Camera.h"

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

        auto player_transform = World::registry->try_get<Transform>(Player::ent);
        auto player_move = World::registry->try_get<Move>(Player::ent);
        auto player_character = World::registry->try_get<Character>(Player::ent);

        for (auto ent : World::registry->view<Portal>())
        {
            auto portal = World::registry->try_get<Portal>(ent);
            if (portal->pn == World::TransPort::tn)
            {
                auto portal_transform = World::registry->try_get<Transform>(ent);
                player_transform->position = SDL_FPoint{portal_transform->position.x, portal_transform->position.y - 10};
                player_move->foo = nullptr;
                player_move->vspeed = 0;
                player_move->hspeed = 0;
                player_character->state = Character::State::JUMP;
                player_character->action = Character::ACTION::JUMP;
                player_character->action_frame = 0;
                player_character->action_index = 0;
                player_character->action_time = 0;
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
        World::registry->destroy(ent);
    }
    World::destory.clear();
}
