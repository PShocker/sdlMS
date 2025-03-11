#include "Attack.h"
#include "Collision.h"
#include "Hit.h"
#include "Core/Core.h"

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
            // 触发攻击效果
            atk->mobCount -= 1;
            // 如果 mobCount 减到 0，提前退出
            if (atk->mobCount < 0)
            {
                return;
            }
            hit_hit(atk, attack_entity, mob_entity, std::nullopt);
        }
    }
}

void attack_reactor(Attack *atk)
{
    if (World::registry->all_of<Skill>(Player::ent) || atk->mobCount <= 0)
    {
        return;
    }
    auto player_transform = World::registry->try_get<Transform>(Player::ent);
    for (auto ent : World::registry->view<Damage, Reactor>())
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
                atk->damage = 1;
                hit_hit(atk, Player::ent, ent, std::nullopt);
                reactor->hit = true;
                return;
            }
        }
    }
}