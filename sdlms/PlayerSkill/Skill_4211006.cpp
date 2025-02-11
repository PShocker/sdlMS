#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 金钱炸弹
int skill_4211006()
{
    auto mv = World::registry->try_get<Move>(Player::ent);
    auto cha = World::registry->try_get<Character>(Player::ent);
    auto state = cha->state;

    // 通用攻击技能
    if (state == Character::State::CLIMB)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP)
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(Player::ent, u"4211006");

    SkillWarp::cooldowns[u"4211006"] = 500;

    auto view = World::registry->view<Drop>();
    for (auto ent : view)
    {
        auto dr = &view.get<Drop>(ent);
        if (dr->id == u"09000000")
        {
            auto d_tr = World::registry->try_get<Transform>(ent);
            auto p_tr = World::registry->try_get<Transform>(Player::ent);
            if (std::abs(p_tr->position.y - d_tr->position.y) <= 10)
            {
                if ((p_tr->flip == 1 && d_tr->position.x > p_tr->position.x && d_tr->position.x - p_tr->position.x < 200) ||
                    (p_tr->flip == 0 && d_tr->position.x < p_tr->position.x && p_tr->position.x - d_tr->position.x < 200))
                {
                    dr->destory = Window::dt_now + 700;
                    auto eff = World::registry->try_get<Effect>(Player::ent);
                    eff->effects.push_back({new Transform(d_tr->position.x, d_tr->position.y),
                                            AnimatedSprite(ski->ski->hits[0]),
                                            Window::dt_now + 700});
                    if (ski->attack == false)
                    {
                        auto lt = ski->ski->infos[ski->level].lt;
                        auto rb = ski->ski->infos[ski->level].rb;
                        auto hit = ski->ski->hits[0];
                        auto mobCount = ski->ski->infos[ski->level].mobCount;
                        auto attackCount = 1;
                        SoundWarp *souw = nullptr;
                        if (ski->ski->sounds.contains(u"Hit"))
                        {
                            souw = ski->ski->sounds[u"Hit"];
                        }
                        ski->atkw = AttackWarp(lt, rb, hit, mobCount, attackCount, souw);
                        ski->attack = true;
                    }
                    ski->atkw.value().damage += dr->nums;
                    continue;
                }
            }
        }
    }
    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT | PlayerSkill::SkillResult::ALERT;
}