#include "Common.h"
#include "Systems/Collision.h"

// 独立函数：寻找最近的可攻击怪物
entt::entity find_closest_attackable_mob(
    int flip,
    const SDL_FPoint &origin,
    const std::unordered_set<entt::entity> &hit_entities,
    float max_x_distance,
    float max_y_distance)
{
    entt::entity closest_mob = entt::null;
    float min_sq_dist = std::numeric_limits<float>::max();

    for (auto e : World::registry->view<Damage, Mob>())
    {
        if (hit_entities.contains(e))
            continue;

        const auto *mob = World::registry->try_get<Mob>(e);
        if (mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
            continue;

        const auto tr = World::registry->try_get<Transform>(e);

        // 快速空间过滤
        const float dx = std::abs(tr->position.x - origin.x);
        const float dy = std::abs(tr->position.y - origin.y);
        if (dx > max_x_distance || dy > max_y_distance)
            continue;
        if ((flip == 1 && tr->position.x < origin.x) || (flip == 0 && tr->position.x > origin.x))
            continue;

        // 计算平方距离
        const float sq_dist = squared_distance(tr->position, origin);
        if (sq_dist < min_sq_dist)
        {
            min_sq_dist = sq_dist;
            closest_mob = e;
        }
    }
    return closest_mob;
}

entt::entity find_closest_attackable_mob(Transform &origin, const Triangle &tri)
{
    entt::entity closest_mob = entt::null;

    float min_sq_dist = std::numeric_limits<float>::max();

    for (auto e : World::registry->view<Damage, Mob>())
    {
        auto mob = World::registry->try_get<Mob>(e);
        if (mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
            continue;

        auto tr = World::registry->try_get<Transform>(e);
        if (!collision(tri, &origin, mob->rect(), tr))
            continue;

        // 计算平方距离
        const float sq_dist = squared_distance(tr->position, origin.position);
        if (sq_dist < min_sq_dist)
        {
            min_sq_dist = sq_dist;
            closest_mob = e;
        }
    }
    return closest_mob;
}

void push_mob_special_effect(entt::entity ent, std::u16string id, AnimatedSpriteWarp *asprw)
{
    auto eff = World::registry->try_get<Effect>(ent);
    if (!eff->effects.contains(id))
    {
        const auto mob_tr = World::registry->try_get<Transform>(ent);
        const auto mob = World::registry->try_get<Mob>(ent);
        auto head = mob->head(mob_tr->flip);
        Effect::Info info;
        info.tr = Transform(SDL_FPoint{0, head.y - 10});
        info.aspr = AnimatedSprite(asprw);
        info.follow = true;
        eff->effects.emplace(id, info);
    }
}