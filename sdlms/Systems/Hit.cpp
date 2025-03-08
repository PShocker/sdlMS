#include "Hit.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Core/Core.h"

void hit_hit(Attack *atk,
             entt::entity src, entt::entity target,
             char type, std::optional<SDL_FPoint> p)
{
    if (!World::registry->valid(target))
    {
        return;
    }
    if (atk->call_back.has_value() && !atk->call_back.value()(src, target))
    {
        return;
    }
    auto hit = World::registry->try_get<Hit>(target);
    for (int i = 0; i < atk->attackCount; i++)
    {
        HitWarp hitw;
        hitw.p = p;
        hitw.asprw = atk->hit;
        hitw.x = atk->p->x;
        hitw.y = atk->p->y;
        hitw.souw = atk->souw;
        // 生成0到1之间的随机浮动因子
        double random_factor = (std::rand() % 101) / 100.0;
        random_factor = hitw.range + (random_factor * 0.1);
        hitw.damage = atk->damage * random_factor;
        hitw.owner = Player::ent;
        hit->hits.push_back(hitw);
    }
}
