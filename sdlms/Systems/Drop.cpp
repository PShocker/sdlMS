#include "Drop.h"
#include "Move.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include <SDL3/SDL.h>
#include "entt/entt.hpp"
#include <vector>

void drop_run()
{
    auto view = World::registry->view<Drop>();
    for (auto ent : view)
    {
        auto dro = &view.get<Drop>(ent);
        if (dro->destory > Window::dt_now)
        {
            auto mv = World::registry->try_get<Move>(ent);
            auto tr = World::registry->try_get<Transform>(ent);
            if (dro->land == false)
            {
                if (!drop_fall(mv, tr, (float)Window::delta_time / 1000))
                {
                    dro->land = true;
                    dro->land_y = tr->position.y;

                    mv->ry0 = dro->land_y - 2.5;
                    mv->ry1 = dro->land_y + 2.5;

                    int random = std::rand() % 2;
                    switch (random)
                    {
                    case 0:
                        mv->vspeed = 3;
                        break;
                    case 1:
                        mv->vspeed = -3;
                        break;
                    }
                }
            }
            else if (dro->picker != entt::null)
            {
                mv->ry0 = std::nullopt;
                mv->ry1 = std::nullopt;

                drop_pick(mv, tr, dro, (float)Window::delta_time / 1000);
            }
            else
            {
                tr->position.y += mv->vspeed * Window::delta_time / 1000;
                if (mv->ry0.has_value() && mv->ry1.has_value())
                {
                    if (tr->position.y < mv->ry0.value() || tr->position.y > mv->ry1.value())
                    {
                        mv->vspeed = -mv->vspeed;
                        tr->position.y = std::clamp(tr->position.y, (float)mv->ry0.value(), (float)mv->ry1.value());
                    }
                }
            }
        }
        else
        {
            World::destory.push_back(ent);
            World::zindex = true;
        }
    }
}

bool drop_fall(Move *mv, Transform *tr, float delta_time)
{
    mv->vspeed += delta_time * 1000;
    return move_fall(mv, tr, delta_time, tr->z % LAYER_Z);
}

void drop_pick(Move *mv, Transform *tr, Drop *dro, float delta_time)
{
    // 默认重力为2000
    // 捡起物品有两阶段,分别为物品向上运动和吸引物品
    auto picker_tr = World::registry->try_get<Transform>(dro->picker);
    auto picker_tr_x = picker_tr->position.x - 10;
    float dx = picker_tr_x - std::lerp(tr->position.x, picker_tr_x, 0.2);

    mv->hspeed = dx / delta_time / 6;

    if (dro->attract)
    {
        auto dy = picker_tr->position.y - tr->position.y;
        mv->vspeed = dy / delta_time / 40;
    }
    else
    {
        mv->vspeed += delta_time * 2000;
        if (mv->vspeed >= 0)
        {
            dro->attract = true;
        }
    }
    move_fall(mv, tr, delta_time, DROP_Z);
    return;
}