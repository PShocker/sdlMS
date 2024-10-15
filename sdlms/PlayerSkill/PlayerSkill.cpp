module;

#include <functional>
#include "entt/entt.hpp"

module playerskill;

import components;
import commons;
import core;

void PlayerSkill::init()
{
    PlayerSkill::Skills[u"2201002"] = skill_2201002;
    PlayerSkill::Skills[u"4211006"] = skill_4211006;
}

void PlayerSkill::skill_sound(SkillWarp *souw)
{
    // 技能音效
    Sound sou;
    sou.souw = souw->sounds[u"Use"];
    Sound::sound_list.push_back(sou);
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

void PlayerSkill::skill_attack(Skill *ski, int damage)
{
    auto ent = Player::ent;
    auto atk = World::registry->try_get<Attack>(ent);
    auto lt = ski->ski->infos[ski->level]->lt;
    auto rb = ski->ski->infos[ski->level]->rb;
    AttackWarp atkw;
    atkw.rect.x = lt.x;
    atkw.rect.y = lt.y;
    atkw.rect.w = rb.x - lt.x;
    atkw.rect.h = rb.y - lt.y;
    atkw.hit = ski->ski->hits[0];
    atkw.mobCount = ski->ski->infos[ski->level]->mobCount;
    atkw.damage = damage;
    atk->atks.push_back(atkw);
}
