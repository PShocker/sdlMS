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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"1121006");

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto attackCount = 1;
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto hit = nullptr;
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];

    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 20);
    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        auto ski = World::registry->try_get<Skill>(ent);
        auto o_mv = World::registry->try_get<Move>(ent);
        auto o_tr = World::registry->try_get<Transform>(ent);
        if (o_mv->foo)
        {
            o_mv->hspeed = o_tr->flip == 1 ? 100 : -100;
            o_mv->hforce = o_tr->flip == 1 ? 1400 : -1400;
            move_move(o_mv, o_tr, 800, 0.06, false);
            o_mv->hspeed = 0;
            auto o_cha = World::registry->try_get<Character>(ent);
            if (o_cha->invincible_cooldown <= 250)
            {
                o_cha->invincible_cooldown = 250;
            }
        }
        // 推动怪物
        for (auto e : World::registry->view<Damage, Mob>())
        {
            const auto mob = World::registry->try_get<Mob>(e);
            if (mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
                continue;
            auto m_tr = World::registry->try_get<Transform>(e);
            if (std::abs(o_tr->position.y - m_tr->position.y) <= 20)
            {
                if ((o_tr->flip == 1 && o_tr->position.x <= m_tr->position.x && (m_tr->position.x - o_tr->position.x) <= 45) ||
                    (o_tr->flip == 0 && o_tr->position.x >= m_tr->position.x && (o_tr->position.x - m_tr->position.x) <= 45))
                {
                    if (!ski->hit_targets.contains(e))
                    {
                        Attack atk;
                        atk.damage = 50;
                        attack_mob(&atk, ent, e, std::nullopt);
                        ski->hit_targets.insert(e);
                    }
                    auto call_back = [x = m_tr->position.x, flip = o_tr->flip](entt::entity ent)
                    {
                        auto m_mv = World::registry->try_get<Move>(ent);
                        if (m_mv->foo)
                        {
                            auto m_tr = World::registry->try_get<Transform>(ent);
                            m_mv->hspeed = flip == 1 ? 145 : -145;
                            m_mv->hforce = flip == 1 ? 1400 : -1400;
                            auto r = move_move(m_mv, m_tr, 800, 0.025, false);
                            m_mv->hforce = 0;
                            if (std::abs(m_tr->position.x - x) >= 200 || r == false || m_mv->hspeed == 0)
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
                    mob->call_backs.emplace(u"1121006", call_back);
                }
            }
        }
    };

    cha->invincible_cooldown = 250;

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}