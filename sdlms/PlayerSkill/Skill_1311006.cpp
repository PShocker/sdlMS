#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Camera.h"
// 龙咆哮
int skill_1311006()
{
    auto ent = Player::ent;
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
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
    // 龙咆哮需要震动摄像机
    camera_init_shake(12, 120, 1060);
    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK | PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}