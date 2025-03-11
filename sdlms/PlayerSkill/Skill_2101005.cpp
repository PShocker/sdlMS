#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>

// 毒雾术
int skill_2101005(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2101005");

    auto lt = SDL_FPoint{0, 0};
    auto rb = SDL_FPoint{0, 0};
    auto hit = ski->skiw->hits[0];
    auto mobCount = 1;
    auto attackCount = 1;
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 50);

    ski->atk.value().call_back = [asprw = AnimatedSpriteWarp::load(ski->skiw->node->find_from_path(u"mob"))](entt::entity src,
                                                                                                             entt::entity target)
    {
        const auto mob = World::registry->try_get<Mob>(target);
        auto call_back = [asprw,
                          time = Window::dt_now + 5000](entt::entity ent)
        {
            const auto mob = World::registry->try_get<Mob>(ent);
            if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
            {
                // 中毒效果
                const auto mob_tr = World::registry->try_get<Transform>(ent);

                bool r = true;
                auto eff = World::registry->try_get<Effect>(ent);
                for (auto &it : eff->effect_list)
                {
                    if (it.aspr.asprw == asprw)
                    {
                        r = false;
                        break;
                    }
                }
                if (r)
                {
                    auto head = mob->head(mob_tr->flip);

                    eff->effect_list.push_back({new Transform(mob_tr->position + SDL_FPoint{0, head.y - 5}),
                                                AnimatedSprite(asprw),
                                                Window::dt_now});
                }
                return std::make_pair(false, true);
            }
            else
            {
                return std::make_pair(true, true);
            }
        };
        mob->call_back_list.push_back(call_back);
        return true;
    };
    ski->ball = 1;

    SkillWarp::cooldowns[u"2101005"] = 500;

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}