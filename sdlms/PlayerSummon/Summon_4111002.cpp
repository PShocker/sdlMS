#include "PlayerSummon.h"
#include "PlayerSkill/PlayerSkill.h"

int summon_4111002(entt::entity ent)
{
    auto sum = World::registry->try_get<Summon>(ent);
    if (World::registry->valid(sum->owner))
    {
        auto owner_cha = World::registry->try_get<Character>(sum->owner);
        auto summon_cha = World::registry->try_get<Character>(ent);

        summon_cha->state = owner_cha->state;
        summon_cha->action = owner_cha->action;
        summon_cha->action_str = owner_cha->action_str;
        summon_cha->action_index = owner_cha->action_index;
        summon_cha->action_frame = owner_cha->action_frame;
        summon_cha->action_time = owner_cha->action_time;
        summon_cha->invincible_cooldown = Window::dt_now + 150;

        summon_cha->alpha = 178;

        auto owner_tr = World::registry->try_get<Transform>(sum->owner);
        auto summon_tr = World::registry->try_get<Transform>(ent);
        summon_tr->flip = owner_tr->flip;
        summon_tr->z = owner_tr->z - 1;
        auto owner_mv = World::registry->try_get<Move>(sum->owner);
        auto summon_mv = World::registry->try_get<Move>(ent);
        summon_mv->foo = owner_mv->foo;
        summon_mv->lr = owner_mv->lr;

        if (owner_mv->lr != nullptr && owner_mv->hspeed == 0 && (owner_cha->action == Character::Action::LADDER || owner_cha->action == Character::Action::ROPE))
        {
            summon_tr->position.x = owner_tr->position.x;
            summon_tr->position.y = owner_tr->position.y + 45;
        }
        else
        {
            if (owner_tr->flip)
            {
                summon_tr->position.x = owner_tr->position.x - 45;
                summon_tr->position.y = owner_tr->position.y;
            }
            else
            {
                summon_tr->position.x = owner_tr->position.x + 45;
                summon_tr->position.y = owner_tr->position.y;
            }
        }
        Skill *owner_ski = World::registry->try_get<Skill>(sum->owner);
        if (owner_ski != nullptr)
        {
            if (owner_cha->action_index == 0 && owner_cha->action_time == 0 && owner_cha->action_frame == 0)
            {
                auto summon_ski = &World::registry->emplace_or_replace<Skill>(ent, owner_ski->skiw->id);
                auto id = summon_ski->skiw->id;
                int skill_res = -1;
                if (PlayerSkill::Skills.contains(id))
                {
                    skill_res = PlayerSkill::Skills.at(id)(ent);
                    if (skill_res == -1)
                    {
                        return false;
                    }
                }
                // 技能效果
                if (skill_res & PlayerSkill::SkillResult::EFF)
                {
                    PlayerSkill::skill_effect(summon_ski, ent);
                }
                // 技能攻击
                if (skill_res & PlayerSkill::SkillResult::ATK)
                {
                    PlayerSkill::skill_attack(summon_ski);
                }
            }
        }
        else
        {
            World::registry->remove<Skill>(ent);
        }
        AfterImage *owner_aft = World::registry->try_get<AfterImage>(sum->owner);
        if (owner_aft != nullptr)
        {
            if (owner_cha->action_index == 0 && owner_cha->action_time == 0 &&
                owner_cha->action_frame == 0 && owner_aft->hit == false)
            {
                auto summon_aft = &World::registry->emplace_or_replace<AfterImage>(ent);
                summon_aft->aspr = owner_aft->aspr;
                summon_aft->wrap = owner_aft->wrap;
            }
        }
    }
    return 0;
}
