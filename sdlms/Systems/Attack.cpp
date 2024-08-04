module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module systems;

import core;
import components;

void attack_run()
{
    auto view = World::registry->view<Attack>();
    for (auto &ent : view)
    {
        auto atk = &view.get<Attack>(ent);
        if (ent == Player::ent)
        {
            auto tr = World::registry->try_get<Transform>(ent);
            attack_player(atk, tr);
        }
    }
    World::registry->remove<Attack>(Player::ent);
}

void attack_player(Attack *atk, Transform *player_tr)
{
    if (atk->x == std::nullopt && atk->y == std::nullopt)
    {
        auto player_rect = atk->rect;
        player_rect.x += player_tr->position.x;
        player_rect.y += player_tr->position.y;
        if (player_tr->flip == 1)
        {
            player_rect.x += 2 * (player_tr->position.x - player_rect.x) - player_rect.w;
        }
        auto view = World::registry->view<Mob>();
        for (auto &ent : view)
        {
            auto mob = &view.get<Mob>(ent);
            auto mob_tr = World::registry->try_get<Transform>(ent);
            auto animated = mob->a[mob->index];
            auto spr = animated->aspr->sprites[animated->anim_index];
            auto lt = spr->lt.value();
            auto rb = spr->rb.value();
            SDL_FRect mob_rect;
            mob_rect.x = lt.x;
            mob_rect.y = lt.y;
            mob_rect.w = rb.x - lt.x;
            mob_rect.h = rb.y - lt.y;
            mob_rect.x += mob_tr->position.x;
            mob_rect.y += mob_tr->position.y;
            if (mob_tr->flip == 1)
            {
                mob_rect.x += 2 * (mob_tr->position.x - mob_rect.x) - mob_rect.w;
            }
            SDL_FRect intersection;
            if (SDL_GetRectIntersectionFloat(&mob_rect, &player_rect, &intersection))
            {
                auto &hit = World::registry->emplace_or_replace<Hit>(ent);
                hit.x = player_tr->position.x;
                hit.y = player_tr->position.y;

                auto &dam = World::registry->emplace_or_replace<Damage>(ent);
                dam.damage.push_back(std::rand() % 99 + 1);

                auto eff = World::registry->try_get<Effect>(ent);
                eff->effects.push_back(AnimatedSprite(atk->hit));
            }
        }
    }
}
