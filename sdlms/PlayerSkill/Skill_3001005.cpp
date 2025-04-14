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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 3001005);

    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->skiw->hits[0];
    auto mobCount = 1;
    auto attackCount = 1;
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 80);

    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        auto ski = World::registry->try_get<Skill>(ent);
        auto cha = World::registry->try_get<Character>(ent);
        auto weaponWrap = World::registry->try_get<WeaponWrap>(ent);
        auto index = AfterImage::afterimages[weaponWrap->afterImage][weaponWrap->afterImage_index][cha->action].index;
        if (!(action_frame == index && action_time == 0))
        {
            return;
        }
        auto point = World::registry->try_get<Transform>(ent)->position + SDL_FPoint{0, -30};
        auto ball_ent_m = load_ball(ent, point, 700, ski);
        auto ball_ent_n = load_ball(ent, point, 700, ski);

        ball_fall(ball_ent_m);
        ball_fall(ball_ent_n);

        auto ball_m = World::registry->try_get<Ball>(ball_ent_m);
        auto ball_n = World::registry->try_get<Ball>(ball_ent_n);

        ball_m->target_point.value().y -= 7.5;
        ball_n->target_point.value().y += 7.5;
    };

    Skill::cooldowns[3001005] = Window::dt_now + 500;

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}