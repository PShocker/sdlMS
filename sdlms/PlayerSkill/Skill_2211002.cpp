#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/MobStateMachine.h"
#include "Commons/Commons.h"

// 冰咆哮
int skill_2211002(entt::entity ent)
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

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, u"2211002");

    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto hit = ski->skiw->hits[0];
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto attackCount = 1;
    SoundWarp *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 40);

    ski->atk.value().call_back = [](entt::entity src, entt::entity target, int full_damage)
    {
        const auto mob = World::registry->try_get<Mob>(target);

        const auto mob_tr = World::registry->try_get<Transform>(target);

        mob->call_backs.erase(u"2211002");
        // 冰冻效果,2秒
        auto call_back = [time = Window::dt_now + 2000](entt::entity ent)
        {
            const auto mob = World::registry->try_get<Mob>(ent);
            if (Window::dt_now <= time && mob->state != Mob::State::DIE && mob->state != Mob::State::REMOVE)
            {
                // 默认选择被攻击的第0帧
                auto sprw = mob->a[u"hit1"].asprw->sprites[0];
                mob->state = Mob::State::HIT;
                mob->index = u"hit1";
                mob->a[u"hit1"].anim_time = 0;
                mob->mod = SDL_Color{90, 148, 247};
                mob_fall(ent, Window::delta_time);
                return std::make_pair(false, false);
            }
            else
            {
                mob->mod = SDL_Color{255, 255, 255};
                return std::make_pair(true, true);
            }
        };
        mob->call_backs.emplace(u"2211002", call_back);
    };

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK | PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}