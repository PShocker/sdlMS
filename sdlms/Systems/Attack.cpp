#include "Attack.h"
#include "Collision.h"
#include "Core/Core.h"
#include "Systems/MobStateMachine.h"
#include "Systems/PlayerStateMachine.h"

void attack_mob(Attack *atk, entt::entity attack_entity)
{
    // 如果 mobCount <= 0，直接返回
    if (atk->mobCount <= 0)
    {
        return;
    }
    // 获取攻击者的 Transform 组件
    auto attack_transform = World::registry->try_get<Transform>(attack_entity);
    // 设置攻击源坐标
    atk->src_point = attack_transform->position;
    // 所有可攻击的怪物集合
    std::map<float, entt::entity> mobs;
    // 遍历所有带有 Damage 和 Mob 组件的实体
    for (auto mob_entity : World::registry->view<Damage, Mob>())
    {
        auto mob = World::registry->try_get<Mob>(mob_entity);
        if (mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
        {
            continue; // 跳过无效或已死亡的怪物
        }
        // 获取怪物的 Transform 组件
        auto mob_transform = World::registry->try_get<Transform>(mob_entity);
        // 检查碰撞
        if (collision(mob->rect(), mob_transform, atk->rect, attack_transform))
        {
            mobs[squared_distance(attack_transform->position, mob_transform->position)] = mob_entity;
        }
    }
    for (auto &[key, val] : mobs)
    {
        // 触发攻击效果
        atk->mobCount -= 1;
        // 如果 mobCount 减到 0，提前退出
        if (atk->mobCount < 0)
        {
            return;
        }
        attack_mob(atk, attack_entity, val, std::nullopt);
    }
}

void attack_reactor(Attack *atk)
{
    if (World::registry->all_of<Skill>(Player::ent) || atk->mobCount <= 0)
    {
        return;
    }
    auto player_transform = World::registry->try_get<Transform>(Player::ent);
    for (auto ent : World::registry->view<Reactor>())
    {
        auto reactor = World::registry->try_get<Reactor>(ent);
        auto reactor_transform = World::registry->try_get<Transform>(ent);

        if (reactor->hit)
        {
            continue;
        }

        for (auto &event : reactor->a[reactor->index].event)
        {
            if (event.type != 0)
            {
                continue;
            }

            if (std::abs(player_transform->position.y - reactor_transform->position.y) > 50)
            {
                continue;
            }

            float distance_x = std::abs(player_transform->position.x - reactor_transform->position.x);
            if (distance_x > 95)
            {
                continue;
            }
            if ((player_transform->flip == 1 && player_transform->position.x <= reactor_transform->position.x) ||
                (player_transform->flip == 0 && player_transform->position.x >= reactor_transform->position.x))
            {
                Sound::push(reactor->sounds[reactor->index]);
                reactor->hit = true;
                return;
            }
        }
    }
}

bool attack_mob(Attack *atk, entt::entity src, entt::entity target, std::optional<SDL_FPoint> p)
{
    if (!World::registry->valid(target))
    {
        return false;
    }
    bool r = false;
    if (auto mob = World::registry->try_get<Mob>(target))
    {
        if (mob->state != Mob::State::REMOVE)
        {
            if (mob_hit(atk, target, p))
            {
                r = true;
            }
        }
    }
    if (r)
    {
        if (atk->call_back.has_value())
        {
            atk->call_back.value()(src, target);
        }
    }
    return r;
}

bool attack_player(Attack *atk, entt::entity src, entt::entity target, std::optional<SDL_FPoint> p)
{
    if (!World::registry->valid(target))
    {
        return false;
    }
    bool r = false;
    if (auto character = World::registry->try_get<Character>(target))
    {
        if (character->state != Character::State::DIE && character->invincible_cooldown <= Window::dt_now)
        {
            if (player_hit(atk, Player::ent))
            {
                r = true;
            }
        }
    }
    if (r)
    {
        if (atk->call_back.has_value())
        {
            atk->call_back.value()(src, target);
        }
    }
    return r;
}