#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <numbers>
#include "Commons/Commons.h"

// 斗气
int skill_1111002(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 1111002);

    auto eff = World::registry->try_get<Effect>(ent);
    eff->effects.emplace(0, Effect::Wrap{std::nullopt, AnimatedSprite(ski->skiw->effects[0])});

    ski->call_back = [n = ski->skiw->node](entt::entity ent, int action_frame, int action_time)
    {
        if (!(action_time == 0 && action_frame == 1))
        {
            return;
        }

        // 持续时间
        const unsigned int duration = 0;

        auto buff = World::registry->try_get<Buff>(ent);
        if (buff->buffs.contains(1111002))
        {
            buff->buffs.at(1111002).destory = Window::dt_now;
        }
        else
        {
            Buff::Wrap wrap;
            wrap.start = [](entt::entity src)
            {
                auto buff = World::registry->try_get<Buff>(src);
                auto &wrap = buff->buffs[1111002];
                wrap.data = std::map<int, float>();
                // 序号,角度
                auto maps = std::any_cast<std::map<int, float>>(&wrap.data);
                maps->emplace(0, 0);
            };
            wrap.after_attack = [](Attack *atk, entt::entity src, entt::entity target, int full_damage)
            {
                auto buff = World::registry->try_get<Buff>(src);
                auto &wrap = buff->buffs[1111002];
                auto maps = std::any_cast<std::map<int, float>>(&wrap.data);
                auto size = maps->size();
                if (size < 6)
                {
                    if (size == 1)
                    {
                        maps->emplace(1, 90);
                    }
                    else
                    {
                        auto angle = maps->at(size - 1);
                        maps->emplace(size, angle + 72);
                    }
                }
            };
            wrap.frame = [n](entt::entity src)
            {
                auto eff = World::registry->try_get<Effect>(src);
                eff->effects.erase(1111002);

                auto buff = World::registry->try_get<Buff>(src);
                auto &wrap = buff->buffs[1111002];
                auto maps = std::any_cast<std::map<int, float>>(&wrap.data);
                auto src_tr = World::registry->try_get<Transform>(src);
                auto src_position = src_tr->position;
                for (auto &[key, val] : *maps)
                {
                    const float ANGLE_INCREMENT = 2.5; // 角速度
                    val += ANGLE_INCREMENT;
                    if (val >= 360.0)
                    {
                        val -= 360.0; // 确保角度在 0~360 度之间
                    }
                    if (key > 0)
                    {
                        AnimatedSprite aspr(n->find_from_path("state/" + std::to_string(key)));
                        aspr.animate = false;
                        auto x = 42 * std::cos(val * std::numbers::pi / 180.0);      // 更新 x 坐标
                        auto y = 42 * std::sin(val * std::numbers::pi / 180.0) - 30; // 更新 y 坐标
                        Transform tr(src_position + SDL_FPoint{(float)x, (float)y});
                        eff->effects.emplace(1111002, Effect::Wrap{tr, aspr});
                    }
                    else
                    {
                        // key==0
                        AnimatedSprite aspr(n->find_from_path(u"state/0"));
                        aspr.animate = false;
                        Transform tr(src_position + SDL_FPoint{0, -30});
                        tr.rotation = val;
                        eff->effects.emplace(1111002, Effect::Wrap{tr, aspr});
                    }
                }
            };
            wrap.finish = [](entt::entity src)
            {
                auto eff = World::registry->try_get<Effect>(src);
                eff->effects.erase(1111002);
            };
            wrap.duration = duration;
            wrap.destory = 0;
            buff->buffs.emplace(1111002, wrap);
            wrap.start.value()(ent);
        }
    };

    if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ACT;
}