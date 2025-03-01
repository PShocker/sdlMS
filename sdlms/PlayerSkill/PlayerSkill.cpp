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
    else if (auto action_str = ski->skiw->level[ski->level]->get_child(u"action"))
    {
        // 从level获取action
        cha->action_str = dynamic_cast<wz::Property<wz::wzstring> *>(action_str)->get();
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
        auto lt = SDL_FPoint{0, 0};
        auto rb = SDL_FPoint{0, 0};
        auto node = ski->skiw->level[ski->level];
        if (ski->skiw->node->get_child(u"ball") == nullptr)
        {
            if (node->get_child(u"lt") && node->get_child(u"rb"))
            {
                auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
                lt = SDL_FPoint{(float)v.x, (float)v.y};
                v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
                rb = SDL_FPoint{(float)v.x, (float)v.y};
            }
            else if (node->get_child(u"range"))
            {
                auto range = dynamic_cast<wz::Property<int> *>(node->get_child(u"range"))->get();
                lt = SDL_FPoint{-(float)range, -60};
                rb = SDL_FPoint{0, 0};
            }
            else
            {
                lt = SDL_FPoint{-110, -32};
                rb = SDL_FPoint{-40, -11};
            }
        }
        AnimatedSpriteWarp *hit = (ski->skiw->hits.size() > 0) ? ski->skiw->hits[0] : nullptr;
        auto mobCount = 1;
        if (node->get_child(u"mobCount"))
        {
            mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
        }
        auto attackCount = 1;
        if (node->get_child(u"attackCount"))
        {
            attackCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"attackCount"))->get();
        }
        SoundWarp *souw = (ski->skiw->sounds.contains(u"Hit")) ? ski->skiw->sounds[u"Hit"] : nullptr;
        ski->atkw = AttackWarp(lt, rb, hit, mobCount, attackCount, souw);
    }
    if (ski->skiw->node->get_child(u"ball") != nullptr)
    {
        ski->ball = 1;
    }
    else
    {
        ski->attack = true;
    }
}
