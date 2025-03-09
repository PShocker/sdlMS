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
    HitWarp hitw;
    hitw.count = atk->attackCount;
    hitw.damage = atk->damage;
    hitw.p = p;
    hitw.asprw = atk->hit;
    hitw.x = atk->p->x;
    hitw.y = atk->p->y;
    hitw.souw = atk->souw;
    hitw.owner = Player::ent;
    hit->hits.push_back(hitw);
}