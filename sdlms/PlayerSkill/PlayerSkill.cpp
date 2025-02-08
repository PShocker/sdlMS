#include "PlayerSkill.h"
#include "Core/Core.h"
#include <functional>
#include "entt/entt.hpp"

void PlayerSkill::init()
{
    PlayerSkill::Skills[u"1311006"] = skill_1311006;
    PlayerSkill::Skills[u"2201002"] = skill_2201002;
    PlayerSkill::Skills[u"2301002"] = skill_2301002;
    PlayerSkill::Skills[u"4001344"] = skill_4001344;
    PlayerSkill::Skills[u"4211006"] = skill_4211006;
}

void PlayerSkill::skill_sound(SkillWarp *souw, int delay)
{
    // 技能音效
    Sound::push(souw->sounds[u"Use"], delay);
}

void PlayerSkill::skill_effect(Skill *ski)
{
    auto ent = Player::ent;
    auto eff = World::registry->try_get<Effect>(ent);

    for (auto &it : ski->ski->effects)
    {
        eff->effects.push_back({nullptr, AnimatedSprite(it)});
    }
}

void PlayerSkill::skill_action(Skill *ski)
{
    auto ent = Player::ent;
    auto cha = World::registry->try_get<Character>(ent);

    cha->state = Character::State::SKILL;
    if (ski->ski->action_str.has_value())
    {
        cha->action_str = ski->ski->action_str.value();
    }
    else
    {
        // 随机动作
        cha->action_str = u"";
    }
    cha->animated = false;
}

void PlayerSkill::skill_attack(Skill *ski)
{
    if (ski->ski->ball != nullptr)
    {
        if (!ski->atkw.has_value())
        {
            auto lt = SDL_FPoint{0, 0};
            auto rb = SDL_FPoint{0, 0};
            auto hit = ski->ski->hits[0];
            auto mobCount = 1;
            auto attackCount = 1;
            SoundWarp *souw = nullptr;
            if (ski->ski->sounds.contains(u"Hit"))
            {
                souw = ski->ski->sounds[u"Hit"];
            }
            ski->atkw = AttackWarp(lt, rb, hit, mobCount, attackCount, souw);
            ski->ball = 1;
        }
    }
    else
    {
        if (!ski->atkw.has_value())
        {
            auto lt = ski->ski->infos[ski->level].lt;
            auto rb = ski->ski->infos[ski->level].rb;
            auto hit = ski->ski->hits[0];
            auto mobCount = ski->ski->infos[ski->level].mobCount;
            auto attackCount = ski->ski->infos[ski->level].attackCount;
            SoundWarp *souw = nullptr;
            if (ski->ski->sounds.contains(u"Hit"))
            {
                souw = ski->ski->sounds[u"Hit"];
            }
            ski->atkw = AttackWarp(lt, rb, hit, mobCount, attackCount, souw);
        }
        ski->attack = true;
    }
}
