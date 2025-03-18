#include "PlayerSummon.h"
#include "PlayerSkill/PlayerSkill.h"

int summon_4111002(entt::entity ent)
{
    auto sum = World::registry->try_get<Summon>(ent);
    if (World::registry->valid(sum->owner))
    {
        auto o_cha = World::registry->try_get<Character>(sum->owner);
        if (o_cha->state == Character::State::DIE)
        {
            World::destory.push_back(ent);
            World::zindex = true;
            return 0;
        }
        auto s_cha = World::registry->try_get<Character>(ent);

        s_cha->state = o_cha->state;
        s_cha->action = o_cha->action;
        s_cha->action_str = o_cha->action_str;
        s_cha->action_index = o_cha->action_index;
        s_cha->action_frame = o_cha->action_frame;
        s_cha->action_time = o_cha->action_time;
        s_cha->invincible_cooldown = Window::dt_now + 150;

        s_cha->alpha = (float)(sum->destory - Window::dt_now) / 30000 * 255;

        auto o_tr = World::registry->try_get<Transform>(sum->owner);
        auto s_tr = World::registry->try_get<Transform>(ent);
        s_tr->flip = o_tr->flip;
        s_tr->z = o_tr->z - 1;
        auto o_mv = World::registry->try_get<Move>(sum->owner);
        auto s_mv = World::registry->try_get<Move>(ent);
        s_mv->foo = o_mv->foo;
        s_mv->lr = o_mv->lr;

        if (o_mv->lr != nullptr && o_mv->hspeed == 0 && (o_cha->action == Character::ACTION::LADDER || o_cha->action == Character::ACTION::ROPE))
        {
            s_tr->position.x = o_tr->position.x;
            s_tr->position.y = o_tr->position.y + 45;
        }
        else
        {
            if (o_tr->flip)
            {
                s_tr->position.x = o_tr->position.x - 45;
                s_tr->position.y = o_tr->position.y;
            }
            else
            {
                s_tr->position.x = o_tr->position.x + 45;
                s_tr->position.y = o_tr->position.y;
            }
        }
        Skill *o_ski = World::registry->try_get<Skill>(sum->owner);
        if (o_ski != nullptr)
        {
            if (o_ski->skiw->id == u"4111002")
            {
                return 0;
            }
            if (o_cha->action_index == 0 && o_cha->action_time == 0 && o_cha->action_frame == 0)
            {
                auto s_ski = &World::registry->emplace_or_replace<Skill>(ent, o_ski->skiw->id);
                auto id = s_ski->skiw->id;
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
                    PlayerSkill::skill_effect(s_ski, ent);
                }
                // 技能攻击
                if (skill_res & PlayerSkill::SkillResult::ATK)
                {
                    PlayerSkill::skill_attack(s_ski);
                }
            }
        }
        else
        {
            World::registry->remove<Skill>(ent);
        }
        AfterImage *o_aft = World::registry->try_get<AfterImage>(sum->owner);
        if (o_aft != nullptr)
        {
            if (o_cha->action_index == 0 && o_cha->action_time == 0 &&
                o_cha->action_frame == 0 && o_aft->hit == false)
            {
                auto s_aft = &World::registry->emplace_or_replace<AfterImage>(ent);
                s_aft->aspr = o_aft->aspr;
                s_aft->info = o_aft->info;
            }
        }
    }
    return 0;
}
