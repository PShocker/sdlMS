#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>
#include "Entities/Ball.h"
#include "Systems/Ball.h"
#include "Commons/Commons.h"

// 二连射
int skill_3001005(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"3001005");

    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->skiw->hits[0];
    auto mobCount = 1;
    auto attackCount = 1;
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 50);

    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        auto ski = World::registry->try_get<Skill>(ent);
        auto cha = World::registry->try_get<Character>(ent);
        auto weaponinfo = World::registry->try_get<WeaponInfo>(ent);
        auto afterImage_index = AfterImage::afterImage_index(weaponinfo->reqLevel);
        auto index = AfterImage::afterimages[weaponinfo->afterImage][afterImage_index][cha->action].index;
        if (!(action_frame == index && action_time == 0))
        {
            return;
        }
    };

    SkillWarp::cooldowns[u"3001005"] = Window::dt_now + 500;

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}