#include "PlayerSkill.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Systems/Attack.h"

// 吸血
int skill_14101006(entt::entity ent)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto cha = World::registry->try_get<Character>(ent);
    auto state = cha->state;

    if (state == Character::State::CLIMB)
    {
        return PlayerSkill::SkillResult::None;
    }
    else if (state != Character::State::JUMP && mv->foo != nullptr)
    {
        mv->hspeed = 0;
    }

    auto ski = &World::registry->emplace_or_replace<Skill>(ent, 14101006);
    auto node = ski->skiw->level[ski->level];
    auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
    auto lt = SDL_FPoint{(float)v.x, (float)v.y};
    v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
    auto rb = SDL_FPoint{(float)v.x, (float)v.y};
    auto hit = ski->skiw->hits[0];
    auto mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    auto attackCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"attackCount"))->get();
    Sound::Wrap *souw = ski->skiw->sounds[u"Hit"];
    ski->atk = Attack(lt, rb, hit, mobCount, attackCount, souw, 90);

    ski->atk.value().call_back = [mobCount](entt::entity src, entt::entity target, int full_damage)
    {
        auto ski = World::registry->try_get<Skill>(src);
        auto atk = &ski->atk.value();
        if (mobCount - 1 == atk->mobCount)
        {
            // 说明是首次,用atk->rect.x来记录伤害量
            atk->rect.x = full_damage;
        }
        else if (atk->mobCount == 0 && atk->rect.x == -400)
        {
            atk->rect.x = full_damage;
        }
        else
        {
            atk->rect.x += full_damage;
        }
        if (atk->mobCount == 0)
        {
            Attack attack;
            attack.damage = -atk->rect.x;
            attack_player(&attack, entt::null, std::nullopt);
        }
    };

    Character::body_actions[u"vampire"][1].delay = 450;

    return PlayerSkill::SkillResult::EFF | PlayerSkill::SkillResult::SOU |
           PlayerSkill::SkillResult::ATK | PlayerSkill::SkillResult::ACT |
           PlayerSkill::SkillResult::ALERT;
}