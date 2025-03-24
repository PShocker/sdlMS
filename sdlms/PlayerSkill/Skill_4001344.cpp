#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>
#include "Entities/Ball.h"
#include "Systems/Ball.h"

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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"4001344");

    auto eff = World::registry->try_get<Effect>(ent);
    eff->effects.emplace(u"4001344", Effect::Info{std::nullopt, AnimatedSprite(ski->skiw->node->find_from_path(u"CharLevel/25/effect"))});

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
        if (action_frame < index)
        {
            return;
        }
        else if (ski->data.has_value())
        {
            auto pair = std::any_cast<std::pair<entt::entity, std::vector<unsigned int>>>(&ski->data);
            auto vec = &pair->second;
            if (!vec->empty())
            {
                auto time = vec->back();
                if (Window::dt_now > time)
                {
                    auto e = load_ball(ent, SDL_FPoint{0, -30 + (float)((2 - vec->size()) * 10)}, 850, ski);
                    auto ball = World::registry->try_get<Ball>(e);
                    if (pair->first == entt::null)
                    {
                        auto target = ball_fall(e, ball);
                        pair->first = target;
                    }
                    else
                    {
                        ball->target = pair->first;
                        ball->p = World::registry->try_get<Transform>(e)->position;
                    }
                    vec->pop_back();
                }
            }
        }
        else
        {
            auto delay = cha->stance_delays[cha->action][cha->action_index];
            auto interval = delay / 4;
            ski->data = std::pair<entt::entity, std::vector<unsigned int>>(entt::null, {Window::dt_now + interval, Window::dt_now});
        }
    };

    SkillWarp::cooldowns[u"4001344"] = Window::dt_now + 500;

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::SOU | PlayerSkill::SkillResult::ACT | PlayerSkill::SkillResult::ALERT;
}