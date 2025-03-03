#include "Hit.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Core/Core.h"

void hit_effect(Attack *atk, std::optional<SDL_FPoint> head,
                entt::entity ent, char type, std::optional<SDL_FPoint> p)
{
    if (!World::registry->valid(ent))
    {
        return;
    }
    auto hit = World::registry->try_get<Hit>(ent);
    hit->x = atk->p->x;
    hit->y = atk->p->y;
    hit->souw = atk->souw;
    hit->count += atk->attackCount;
    hit->damage += atk->damage * atk->attackCount;
    hit->owner = Player::ent;

    auto dam = World::registry->try_get<Damage>(ent);
    dam->head = head;
    auto count = dam->damage.size();

    for (int i = 0; i < atk->attackCount; i++)
    {
        Damage::Info info;
        info.damage = std::abs(atk->damage);
        info.alpha = 255;
        info.type = type;
        info.delay = Window::dt_now + (i + (int)count) * 64;
        info.x = (float)(std::rand() % 11 - 5);
        info.y = (float)(i + count) * 38;
        dam->damage.push_back(info);
    }
    if (atk->hit)
    {
        auto eff = World::registry->try_get<Effect>(ent);
        if (p.has_value())
        {
            eff->effects.push_back({new Transform(p.value(), 0, World::registry->try_get<Transform>(ent)->flip),
                                    AnimatedSprite(atk->hit), Window::dt_now});
        }
        else
        {
            eff->effects.push_back({nullptr, AnimatedSprite(atk->hit), Window::dt_now});
        }
    }
    if (atk->call_back.has_value())
    {
        atk->call_back.value()(ent);
    }
}

void hit_effect(Attack *atk, entt::entity ent, char type, std::optional<SDL_FPoint> p)
{
    hit_effect(atk, std::nullopt, ent, type, p);
}
