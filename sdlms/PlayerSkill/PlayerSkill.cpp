#include "PlayerSkill.h"
#include "Core/Core.h"
#include <functional>
#include "entt/entt.hpp"

void PlayerSkill::init()
{
    PlayerSkill::Skills[u"2201002"] = skill_2201002;
    PlayerSkill::Skills[u"2301002"] = skill_2301002;
    // PlayerSkill::Skills[u"4211006"] = skill_4211006;
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
        ski->ball = true;
    }
    else
    {
        ski->attack = true;
    }
}
