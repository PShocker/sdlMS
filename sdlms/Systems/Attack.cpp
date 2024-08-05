module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module systems;

import core;
import components;

void attack_run()
{
    if (auto atk = World::registry->try_get<Attack>(Player::ent))
    {
        atk->damage = std::rand() % 99 + 1;
        auto tr = World::registry->try_get<Transform>(Player::ent);
        player_attack(atk, tr);
        World::registry->remove<Attack>(Player::ent);
    }
    auto view = World::registry->view<Mob>();
    for (auto &ent : view)
    {
        auto mob = &view.get<Mob>(ent);
        if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
        {
            mob_collision(mob, World::registry->try_get<Transform>(ent));
        }
    }
}

void player_attack(Attack *atk, Transform *tr)
{
    if (atk->x == std::nullopt && atk->y == std::nullopt)
    {
        auto atk_rect = atk->rect;
        atk_rect.x += tr->position.x;
        atk_rect.y += tr->position.y;
        if (tr->flip == 1)
        {
            atk_rect.x += 2 * (tr->position.x - atk_rect.x) - atk_rect.w;
        }
        auto view = World::registry->view<Damage>();
        for (auto ent : view)
        {
            if (auto mob = World::registry->try_get<Mob>(ent))
            {
                if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
                {
                    attack_mob(atk, World::registry->try_get<Transform>(ent), mob, &atk_rect, &tr->position, &ent);
                }
            }
            else if (auto npc = World::registry->try_get<Npc>(ent))
            {
                attack_npc(atk, World::registry->try_get<Transform>(ent), npc, &atk_rect, &tr->position, &ent);
            }
        }
    }
}

void mob_collision(Mob *mob, Transform *tr)
{
    auto animated = mob->a[mob->index];
    auto spr = animated->aspr->sprites[animated->anim_index];
    auto lt = spr->lt.value();
    auto rb = spr->rb.value();
    SDL_FRect rect;
    rect.x = lt.x;
    rect.y = lt.y;
    rect.w = rb.x - lt.x;
    rect.h = rb.y - lt.y;
    rect.x += tr->position.x;
    rect.y += tr->position.y;
    if (tr->flip == 1)
    {
        rect.x += 2 * (tr->position.x - rect.x) - rect.w;
    }

    auto player_transform = World::registry->try_get<Transform>(Player::ent);
    auto player_character = World::registry->try_get<Character>(Player::ent);

    SDL_FRect rect1;
    rect1.x = player_character->lt.x;
    rect1.y = player_character->lt.y;
    rect1.w = player_character->rb.x - player_character->lt.x;
    rect1.h = player_character->rb.y - player_character->lt.y;
    rect1.x += player_transform->position.x;
    rect1.y += player_transform->position.y;
    if (player_transform->flip == 1)
    {
        rect1.x += 2 * (player_transform->position.x - rect1.x) - rect1.w;
    }
    SDL_FRect intersection;
    if (SDL_GetRectIntersectionFloat(&rect, &rect1, &intersection))
    {
        auto damage = mob->damage;

        auto &hit = World::registry->emplace_or_replace<Hit>(Player::ent);
        hit.x = tr->position.x;
        hit.y = tr->position.y;
        hit.damage = damage;

        auto dam = World::registry->try_get<Damage>(Player::ent);
        dam->damage.push_back({damage, 255, (int)dam->damage.size()});
    }
}

void attack_mob(Attack *atk, Transform *tr, Mob *mob, SDL_FRect *atk_rect, SDL_FPoint *pos, entt::entity *ent)
{
    auto animated = mob->a[mob->index];
    auto spr = animated->aspr->sprites[animated->anim_index];
    auto lt = spr->lt.value();
    auto rb = spr->rb.value();
    SDL_FRect rect;
    rect.x = lt.x;
    rect.y = lt.y;
    rect.w = rb.x - lt.x;
    rect.h = rb.y - lt.y;
    rect.x += tr->position.x;
    rect.y += tr->position.y;
    if (tr->flip == 1)
    {
        rect.x += 2 * (tr->position.x - rect.x) - rect.w;
    }
    SDL_FRect intersection;
    if (SDL_GetRectIntersectionFloat(&rect, atk_rect, &intersection))
    {
        auto damage = std::max(atk->damage - mob->def, 1);

        auto &hit = World::registry->emplace_or_replace<Hit>(*ent);
        hit.x = pos->x;
        hit.y = pos->y;
        hit.damage = damage;

        auto dam = World::registry->try_get<Damage>(*ent);
        dam->damage.push_back({damage, 255, (int)dam->damage.size()});

        auto eff = World::registry->try_get<Effect>(*ent);
        eff->effects.push_back(AnimatedSprite(atk->hit));
    }
}

void attack_npc(Attack *atk, Transform *tr, Npc *npc, SDL_FRect *atk_rect, SDL_FPoint *pos, entt::entity *ent)
{
    auto animated = npc->a[npc->index];
    auto spr = animated->aspr->sprites[animated->anim_index];
    SDL_FRect rect;
    rect.x = tr->position.x;
    rect.y = tr->position.y;
    rect.w = spr->width;
    rect.h = spr->height;
    SDL_FRect intersection;
    if (SDL_GetRectIntersectionFloat(&rect, atk_rect, &intersection))
    {
        auto damage = atk->damage;

        auto &hit = World::registry->emplace_or_replace<Hit>(*ent);
        hit.x = pos->x;
        hit.y = pos->y;
        hit.damage = damage;

        auto dam = World::registry->try_get<Damage>(*ent);
        dam->damage.push_back({damage, 255, (int)dam->damage.size()});

        auto eff = World::registry->try_get<Effect>(*ent);
        eff->effects.push_back(AnimatedSprite(atk->hit));
    }
}
