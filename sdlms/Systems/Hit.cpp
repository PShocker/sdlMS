#include "Hit.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Core/Core.h"

void hit_effect(AttackWarp *atkw, std::optional<SDL_FPoint> head, entt::entity ent,
                char type, int damage, int count,
                SDL_FPoint *p)
{
    if (!World::registry->valid(ent))
    {
        return;
    }
    
    auto hit = World::registry->try_get<Hit>(ent);
    hit->x = atkw->p->x;
    hit->y = atkw->p->y;
    hit->souw = atkw->souw;
    hit->count += count;
    hit->damage += damage * count;
    hit->owner = Player::ent;

    for (int i = 0; i < count; i++)
    {
        auto dam = World::registry->try_get<Damage>(ent);
        dam->damage.push_back({std::abs(damage), 255, (int)dam->damage.size(), type});
        dam->head = head;

        if (atkw->hit)
        {
            auto eff = World::registry->try_get<Effect>(ent);
            if (p != nullptr)
            {
                eff->effects.push_back({new Transform(*p), AnimatedSprite(atkw->hit), Window::dt_now + i * 145});
            }
            else
            {
                eff->effects.push_back({nullptr, AnimatedSprite(atkw->hit), Window::dt_now + i * 145});
            }
        }
    }
}

void hit_effect(AttackWarp *atkw, Mob *mob, entt::entity ent, SDL_FPoint *p)
{
    hit_effect(atkw, mob->head(), ent, 0, atkw->damage, atkw->attackCount, p);
}

void hit_effect(AttackWarp *atkw, Npc *npc, entt::entity ent, SDL_FPoint *p)
{
    hit_effect(atkw, npc->head(), ent, 2, 1, 1, p);
}

void hit_effect(AttackWarp *atkw, entt::entity ent, SDL_FPoint *p)
{
    hit_effect(atkw, std::nullopt, ent, 1, atkw->damage, atkw->attackCount, p);
}
