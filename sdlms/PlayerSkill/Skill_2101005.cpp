#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include "Common.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include <SDL3/SDL.h>
#include "Systems/Attack.h"

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
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 10);

    ski->atk.value().call_back = [hit, asprw = AnimatedSpriteWarp::load(ski->skiw->node->find_from_path(u"mob"))](entt::entity src,
                                                                                                                  entt::entity target)
    {
        const auto mob = World::registry->try_get<Mob>(target);
        unsigned int time = Window::dt_now + 5000;

        mob->call_backs.erase(u"2101005");

        auto call_back = [asprw,
                          time](entt::entity ent)
        {
            const auto mob = World::registry->try_get<Mob>(ent);

            if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
            {
                // 中毒效果
                push_mob_special_effect(ent, asprw);
                return std::make_pair(false, true);
            }
            else
            {
                return std::make_pair(true, true);
            }
        };
        mob->call_backs.emplace(u"2101005", call_back);

        for (int i = Window::dt_now + 1000; i < time; i += 1000)
        {
            const auto hit_call_back = [hit, i](entt::entity ent)
            {
                if (i <= Window::dt_now)
                {
                    Attack atk;
                    atk.damage = 10;
                    atk.hit = hit;
                    atk.src_point = std::nullopt;
                    attack_mob(&atk, Player::ent, ent, std::nullopt);
                    return std::make_pair(true, true);
                }
                else
                {
                    return std::make_pair(false, true);
                }
            };
            mob->call_backs.emplace(u"2101005", hit_call_back);
        }
    };
    ski->ball = 1;

    SkillWarp::cooldowns[u"2101005"] = 500;

    return PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}