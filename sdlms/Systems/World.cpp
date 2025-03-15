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
        auto view = World::registry->view<Portal>();
        for (auto &e : view)
        {
            auto por = &view.get<Portal>(e);
            if (por->pn == World::TransPort::tn)
            {
                auto tr = World::registry->try_get<Transform>(e);
                Player::ent = World::registry->create();
                load_character(tr->position.x, tr->position.y - 10, false, Player::ent);
                load_pet(u"5000000", Player::ent);
                camera_refresh();
                World::TransPort::id = 0;
                return;
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
