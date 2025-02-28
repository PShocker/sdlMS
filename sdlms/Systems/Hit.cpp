#include "Hit.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Core/Core.h"

void hit_effect(AttackWarp *atkw, std::optional<SDL_FPoint> head,
                entt::entity ent, char type, SDL_FPoint *p)
{
    if (!World::registry->valid(ent))
    {
        return;
    }
    auto hit = World::registry->try_get<Hit>(ent);
    hit->x = atkw->p->x;
    hit->y = atkw->p->y;
    hit->souw = atkw->souw;
    hit->count += atkw->attackCount;
    hit->damage += atkw->damage * atkw->attackCount;
    hit->owner = Player::ent;

    auto dam = World::registry->try_get<Damage>(ent);
    dam->head = head;
    auto count = dam->damage.size();

    for (int i = 0; i < atkw->attackCount; i++)
    {
        Damage::Info info;
        info.damage = std::abs(atkw->damage);
        info.alpha = 255;
        info.type = type;
        info.delay = Window::dt_now + (i + (int)count) * 64;
        info.x = (float)(std::rand() % 11 - 5);
        info.y = (float)(i + count) * 38;
        dam->damage.push_back(info);
    }
    
    if (atkw->hit)
    {
        auto eff = World::registry->try_get<Effect>(ent);
        if (p != nullptr)
        {
            eff->effects.push_back({new Transform(*p), AnimatedSprite(atkw->hit), Window::dt_now});
        }
        else
        {
            eff->effects.push_back({nullptr, AnimatedSprite(atkw->hit), Window::dt_now});
        }
    }
}

void hit_effect(AttackWarp *atkw, entt::entity ent, char type, SDL_FPoint *p)
{
    hit_effect(atkw, std::nullopt, ent, type, p);
}
