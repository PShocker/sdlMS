#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 回旋斩
int skill_4201005(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    // 通用攻击技能
    if (state == Character::State::CLIMB)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    auto weaponinfo = World::registry->try_get<WeaponInfo>(ent);

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"4201005");

    auto lt = SDL_FPoint{-130, -32};
    auto rb = SDL_FPoint{-40, -11};
    // 对这个技能来说，每个动作段数为1，如果段数不足，则最后一个攻击帧补全不足的段数
    auto attackCount = 1;
    auto mobCount = 1;
    auto hit = ski->skiw->hits[0];
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];

    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 10);
    ski->atk.value().call_back = [&atk = ski->atk](entt::entity src, entt::entity target)
    {
        auto cha = World::registry->try_get<Character>(src);
        auto ski = World::registry->try_get<Skill>(src);

        auto node = ski->skiw->level[ski->level];
        auto attackCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"attackCount"))->get();
        auto current_frame = cha->action_frame;

        const int action_frame = 3;
        const int attack_frame = 5;
        int delta = attack_frame - attackCount;
        if (delta > 0)
        {
            // 如果攻击次数小于攻击帧数
            if (current_frame >= action_frame + attack_frame - delta)
            {
                ski->hit = true;
                return false; // 停止攻击
            }
        }
        else if (delta < 0)
        {
            // 如果攻击次数大于攻击帧数
            if (current_frame == action_frame + attack_frame - 1)
            {
                atk->attackCount = 1 - delta;
            }
        }
        // 设置hit为false，表示下一帧继续触发攻击效果
        ski->hit = false;
        return true;
    };
    ski->call_back = [&atk = ski->atk](entt::entity ent)
    {
        atk->mobCount = 1;
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK | PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}