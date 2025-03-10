#include "PlayerSkill.h"
#include "Common.h"
#include "Commons/Commons.h"
#include "Systems/Hit.h"
#include "Systems/MobStateMachine.h"
#include "Components/Components.h"
#include "Entities/Entities.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>

// 爆炸箭
int skill_3101005(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"3101005");
    ski->ball = 1;

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto hit = ski->skiw->hits[0];
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto attackCount = 1;
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw);
    ski->atk.value().call_back = [](entt::entity src, entt::entity target)
    {
        auto ski = World::registry->try_get<Skill>(src);
        auto atk = &ski->atk.value();
        atk->call_back = std::nullopt;
        auto target_position = World::registry->try_get<Transform>(target)->position;

        do
        {
            const auto mob = World::registry->try_get<Mob>(target);
            const auto target_tr = World::registry->try_get<Transform>(target);

            // 执行攻击效果
            const SDL_FPoint hit_point = target_tr->position + mob->head(target_tr->flip);
            hit_hit(atk, src, target, 0, hit_point);

            // 晕眩效果,3秒
            auto call_back = [asprw = AnimatedSpriteWarp::load(ski->skiw->node->find_from_path(u"mob")),
                              time = Window::dt_now + 3000](entt::entity ent)
            {
                const auto mob = World::registry->try_get<Mob>(ent);

                if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
                {
                    // 晕眩特效
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
                    mob_fall(ent, Window::delta_time);
                    return std::make_pair(false, false);
                }
                else
                {
                    return std::make_pair(true, true);
                }
            };
            mob->call_back_list.push_back(call_back);

            ski->hit_targets.insert(target);
            atk->mobCount--;

            // 寻找下一个目标
            target = find_closest_attackable_mob(
                -1,
                target_position,
                ski->hit_targets,
                200.0f, // max_x_distance
                150.0f  // max_y_distance
            );
        } while (World::registry->valid(target) && atk->mobCount > 0);
        return false;
    };

    World::registry->emplace_or_replace<AfterImage>(ent);

    return PlayerSkill::SkillResult::EFF |
           PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK |
           PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}