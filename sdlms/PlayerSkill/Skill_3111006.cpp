#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>
#include "Entities/Ball.h"
#include "Systems/Ball.h"
#include "Commons/Commons.h"

// 箭扫射
int skill_3111006(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 3111006);

    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->skiw->hits[0];
    auto mobCount = 1;
    auto attackCount = 1;
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 50);

    ski->call_back = [](entt::entity ent, int action_frame, int action_time)
    {
        auto ski = World::registry->try_get<Skill>(ent);
        auto cha = World::registry->try_get<Character>(ent);
        auto weaponWrap = World::registry->try_get<WeaponWrap>(ent);
        auto index = AfterImage::afterimages[weaponWrap->afterImage][weaponWrap->afterImage_index][cha->action].index;
        if (action_frame < index)
        {
            return;
        }
        if (ski->data.has_value())
        {
            auto tuple = std::any_cast<std::tuple<entt::entity, std::vector<unsigned int>, SDL_FPoint>>(&ski->data);
            auto vec = &std::get<1>(*tuple);
            if (!vec->empty())
            {
                auto time = vec->back();
                if (Window::dt_now > time)
                {
                    auto point = SDL_FPoint{0, -30};
                    point = point + std::get<2>(*tuple);
                    auto ball_ent = load_ball(ent, point, 700, ski);
                    auto ball = World::registry->try_get<Ball>(ball_ent);
                    if (vec->size() == 4)
                    {
                        auto target = ball_fall(ball_ent);
                        std::get<0>(*tuple) = target;
                    }
                    else
                    {
                        auto mob_ent = std::get<0>(*tuple);
                        ball_target_point(ball_ent, mob_ent);
                    }
                    if (ball->target != entt::null)
                    {
                        auto mob = World::registry->try_get<Mob>(ball->target);
                        ball->target_point = SDL_FPoint{0, -(mob->rect().h / 4) * (float)vec->size()};
                    }
                    else
                    {
                        auto ball_tr = World::registry->try_get<Transform>(ball_ent);
                        auto target_point = ball_tr->flip == 1 ? SDL_FPoint{350, 0} : SDL_FPoint{-350, 0};
                        target_point.y = -36 + (4 - (float)vec->size()) * 18;
                        ball->target_point = ball_tr->position + target_point;
                    }
                    vec->pop_back();
                }
            }
        }
        else
        {
            auto delay = cha->stance_delays[cha->action][cha->action_index];
            auto interval = delay / 6;
            ski->data = std::tuple<entt::entity, std::vector<unsigned int>, SDL_FPoint>(entt::null, {
                                                                                                        Window::dt_now + interval * 3,
                                                                                                        Window::dt_now + interval * 2,
                                                                                                        Window::dt_now + interval,
                                                                                                        Window::dt_now,
                                                                                                    },
                                                                                        World::registry->try_get<Transform>(ent)->position);
        }
    };

    Skill::cooldowns[3111006] = Window::dt_now + 500;

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}