#include "PlayerSkill.h"
#include "Core/Core.h"
#include <functional>
#include "entt/entt.hpp"

void PlayerSkill::skill_sound(SkillWarp *souw, int delay)
{
    // 技能音效
    Sound::push(souw->sounds[u"Use"], delay);
}

void PlayerSkill::skill_effect(Skill *ski, entt::entity ent)
{
    auto eff = World::registry->try_get<Effect>(ent);

    for (auto &it : ski->skiw->effects)
    {
        eff->effects.push_back({nullptr, AnimatedSprite(it)});
    }
}

void PlayerSkill::skill_action(Skill *ski, entt::entity ent)
{
    auto cha = World::registry->try_get<Character>(ent);

    cha->state = Character::State::SKILL;
    if (ski->skiw->action_str.has_value())
    {
        cha->action_str = ski->skiw->action_str.value();
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
    if (!ski->atkw.has_value())
    {
        auto lt = (ski->skiw->ball != nullptr) ? SDL_FPoint{0, 0} : ski->skiw->infos[ski->level].lt;
        auto rb = (ski->skiw->ball != nullptr) ? SDL_FPoint{0, 0} : ski->skiw->infos[ski->level].rb;
        AnimatedSpriteWarp *hit = (ski->skiw->hits.size() > 0) ? ski->skiw->hits[0] : nullptr;
        auto mobCount = ski->skiw->infos[ski->level].mobCount;
        auto attackCount = ski->skiw->infos[ski->level].attackCount;
        SoundWarp *souw = (ski->skiw->sounds.contains(u"Hit")) ? ski->skiw->sounds[u"Hit"] : nullptr;
        ski->atkw = AttackWarp(lt, rb, hit, mobCount, attackCount, souw);
    }
    if (ski->skiw->ball != nullptr)
    {
        ski->ball = 1;
    }
    else
    {
        ski->attack = true;
    }
}
