#include "Pick.h"

entt::entity pick_drop(entt::entity owner)
{
    entt::entity e = entt::null;
    // 捡起物品
    auto owner_tr = World::registry->try_get<Transform>(owner);
    auto owner_x = owner_tr->position.x;
    auto owner_y = owner_tr->position.y;

    for (auto ent : World::registry->view<Drop>())
    {
        auto dro = World::registry->try_get<Drop>(ent);
        if (dro->picker == entt::null && dro->land == true)
        {
            auto dro_tr = World::registry->try_get<Transform>(ent);
            auto dro_x = dro_tr->position.x;
            auto dro_y = dro_tr->position.y;
            if (owner_x == std::clamp(owner_x, dro_x - 20, dro_x + 40) &&
                owner_y == std::clamp(owner_y, dro_y - 20, dro_y + 20))
            {
                // 捡起物品
                dro->picker = owner;
                dro->destory = Window::dt_now + 600;

                auto mv = World::registry->try_get<Move>(ent);
                mv->vspeed = -430;

                // 播放声音
                Sound::push(Sound(u"Game.img/PickUpItem"));
                return e;
            }
        }
    }
    return e;
}
