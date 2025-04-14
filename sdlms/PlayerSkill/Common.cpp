#include "Common.h"
#include "Systems/Collision.h"
#include "Systems/Attack.h"
#include "Systems/MobStateMachine.h"

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

    for (auto e : World::registry->view<Mob>())
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

    for (auto e : World::registry->view<Mob>())
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

void mob_special_effect(entt::entity ent, int id, AnimatedSprite::Wrap *asprw, unsigned int time)
{
    auto eff = World::registry->try_get<Effect>(ent);
    if (!eff->effects.contains(id))
    {
        const auto mob_tr = World::registry->try_get<Transform>(ent);
        const auto mob = World::registry->try_get<Mob>(ent);
        auto head = mob->head(mob_tr->flip);
        eff->effects.insert({id, {Transform(SDL_FPoint{0, head.y - 10}), AnimatedSprite(asprw), Window::dt_now, true, time}});
    }
}

std::pair<bool, bool> dizzy_call_back(entt::entity ent, std::any &data)
{
    const auto mob = World::registry->try_get<Mob>(ent);
    auto time = std::any_cast<unsigned int>(data);

    if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
    {
        auto asprw = AnimatedSprite::Wrap::load(Skill::Wrap::load(3101005)->node->find_from_path(u"mob"));
        // 晕眩特效
        mob_special_effect(ent, Effect::Dizzy, asprw, time);
        mob_fall(ent, Window::delta_time);
        return std::make_pair(false, false);
    }
    else
    {
        auto eff = World::registry->try_get<Effect>(ent);
        eff->effects.erase(Effect::Dizzy);
        return std::make_pair(true, true);
    }
}

std::pair<bool, bool> poison_call_back(entt::entity ent, std::any &data)
{
    const auto mob = World::registry->try_get<Mob>(ent);
    auto [time, atktime] = std::any_cast<std::tuple<unsigned int, unsigned int>>(data);

    if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
    {
        // 中毒特效
        auto asprw = AnimatedSprite::Wrap::load(Skill::Wrap::load(2101005)->node->find_from_path(u"mob"));
        mob_special_effect(ent, Effect::Poison, asprw, time);
        mob->mod = SDL_Color{136, 204, 0};
        if (atktime < Window::dt_now)
        {
            Attack atk;
            atk.damage = 10;
            atk.src_point = std::nullopt;
            attack_mob(&atk, Player::ent, ent, std::nullopt);
            atktime += 1000;
            std::get<1>(std::any_cast<std::tuple<unsigned int, unsigned int> &>(data)) = atktime;
        }
        return std::make_pair(false, true);
    }
    else
    {
        mob->mod = SDL_Color{255, 255, 255};
        auto eff = World::registry->try_get<Effect>(ent);
        eff->effects.erase(Effect::Poison);
        return std::make_pair(true, true);
    }
}

std::pair<bool, bool> flame_call_back(entt::entity ent, std::any &data)
{
    const auto mob = World::registry->try_get<Mob>(ent);
    auto [time, atktime] = std::any_cast<std::tuple<unsigned int, unsigned int>>(data);

    if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
    {
        // 火焰特效
        auto asprw = AnimatedSprite::Wrap::load(Skill::Wrap::load(3111003)->node->find_from_path(u"tile/0"));
        mob_special_effect(ent, Effect::Flame, asprw, time);
        mob->mod = SDL_Color{255, 100, 0};
        if (atktime < Window::dt_now)
        {
            Attack atk;
            atk.damage = 10;
            atk.src_point = std::nullopt;
            attack_mob(&atk, Player::ent, ent, std::nullopt);
            auto &tuple = std::any_cast<std::tuple<unsigned int, unsigned int> &>(data);
            std::get<1>(tuple) = Window::dt_now + 500;
        }
        return std::make_pair(false, true);
    }
    else
    {
        mob->mod = SDL_Color{255, 255, 255};
        auto eff = World::registry->try_get<Effect>(ent);
        eff->effects.erase(Effect::Flame);
        return std::make_pair(true, true);
    }
}

std::pair<bool, bool> frozen_call_back(entt::entity ent, std::any &data)
{
    const auto mob = World::registry->try_get<Mob>(ent);
    auto time = std::any_cast<unsigned int>(data);

    if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
    {
        // 默认选择被攻击的第0帧
        auto sprw = mob->a[u"hit1"].asprw->sprites[0];
        mob->state = Mob::State::HIT;
        mob->index = u"hit1";
        mob->a[u"hit1"].anim_time = 0;
        mob->mod = SDL_Color{90, 148, 247};
        mob_fall(ent, Window::delta_time);
        return std::make_pair(false, false);
    }
    else
    {
        mob->mod = SDL_Color{255, 255, 255};
        return std::make_pair(true, true);
    }
}
