#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Move.h"
#include "Systems/Attack.h"

// 突进
int skill_1121006(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    if (mv->foo == nullptr)
    {
        return PlayerSkill::SkillResult::None;
    }
    else
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 1121006);

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto attackCount = 1;
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto hit = nullptr;
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];

    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 20);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        auto ski = World::registry->try_get<Skill>(ent);
        auto owner_mv = World::registry->try_get<Move>(ent);
        auto owner_tr = World::registry->try_get<Transform>(ent);
        if (owner_mv->foo)
        {
            owner_mv->hspeed = owner_tr->flip == 1 ? 100 : -100;
            owner_mv->hforce = owner_tr->flip == 1 ? 1400 : -1400;
            move_move(owner_mv, owner_tr, 800, 0.06, false);
            owner_mv->hspeed = 0;
            auto owner_cha = World::registry->try_get<Character>(ent);
            if (owner_cha->invincible_cooldown <= Window::dt_now + 250)
            {
                owner_cha->invincible_cooldown = Window::dt_now + 250;
            }
        }
        // 推动怪物
        for (auto mob_ent : World::registry->view<Mob>())
        {
            const auto mob = World::registry->try_get<Mob>(mob_ent);
            if (mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
                continue;
            auto m_tr = World::registry->try_get<Transform>(mob_ent);
            if (std::abs(owner_tr->position.y - m_tr->position.y) <= 20)
            {
                if ((owner_tr->flip == 1 && owner_tr->position.x <= m_tr->position.x && (m_tr->position.x - owner_tr->position.x) <= 45) ||
                    (owner_tr->flip == 0 && owner_tr->position.x >= m_tr->position.x && (owner_tr->position.x - m_tr->position.x) <= 45))
                {
                    if (!ski->hit_targets.contains(mob_ent))
                    {
                        Attack atk;
                        atk.damage = 50;
                        atk.mobCount = 0;
                        attack_mob(&atk, ent, mob_ent, std::nullopt);
                        ski->hit_targets.insert(mob_ent);
                    }
                    auto call_back = [x = m_tr->position.x, flip = owner_tr->flip](entt::entity ent, std::any data)
                    {
                        auto mob_mv = World::registry->try_get<Move>(ent);
                        if (mob_mv->foo)
                        {
                            auto m_tr = World::registry->try_get<Transform>(ent);
                            mob_mv->hspeed = flip == 1 ? 145 : -145;
                            mob_mv->hforce = flip == 1 ? 1400 : -1400;
                            auto r = move_move(mob_mv, m_tr, 800, 0.025, false);
                            mob_mv->hforce = 0;
                            if (std::abs(m_tr->position.x - x) >= 200 || r == false || mob_mv->hspeed == 0)
                            {
                                return std::make_pair(true, true);
                            }
                            else
                            {
                                return std::make_pair(false, false);
                            }
                        }
                        return std::make_pair(true, true);
                    };
                    mob->call_backs.emplace(1121006, std::make_pair(call_back, std::any{}));
                }
            }
        }
    };

    cha->invincible_cooldown = Window::dt_now + 250;

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}