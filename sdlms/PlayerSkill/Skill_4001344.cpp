#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>
#include "Entities/Ball.h"
#include "Systems/Ball.h"
#include "Commons/Commons.h"

// 双飞镖
int skill_4001344(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 4001344);

    auto eff = World::registry->try_get<Effect>(ent);
    eff->effects.emplace(4001344, Effect::Wrap{std::nullopt, AnimatedSprite(ski->skiw->node->find_from_path(u"CharLevel/25/effect"))});

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
                    point.y += std::rand() % 11;
                    point = point + std::get<2>(*tuple);
                    auto ball_ent = load_ball(ent, point, 900, ski);
                    if (vec->size() == 2)
                    {
                        auto target = ball_fall(ball_ent);
                        std::get<0>(*tuple) = target;
                    }
                    else
                    {
                        ball_target_point(ball_ent, std::get<0>(*tuple));
                    }
                    vec->pop_back();
                }
            }
        }
        else
        {
            auto delay = cha->stance_delays[cha->action][cha->action_index];
            auto interval = delay / 2;
            ski->data = std::tuple<entt::entity, std::vector<unsigned int>, SDL_FPoint>(entt::null, {
                                                                                                        Window::dt_now + interval,
                                                                                                        Window::dt_now,
                                                                                                    },
                                                                                        World::registry->try_get<Transform>(ent)->position);
        }
    };

    Skill::cooldowns[4001344] = Window::dt_now + 500;

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT | PlayerSkill::SkillResult::ALERT;
}