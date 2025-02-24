#include "Skill.h"
#include "Sound.h"

#include "wz/Property.hpp"
#include <string>
#include <SDL3/SDL.h>

#include "Resources/Wz.h"

static std::unordered_map<std::u16string, SkillWarp *> cache;

SkillWarp::SkillWarp(const std::u16string &id) : id(id)
{
    auto node = Wz::Skill->get_root()->find_from_path(id.substr(0, id.length() - 4) + u".img/skill/" + id);
    if (node->get_child(u"effect"))
    {
        effects.push_back(AnimatedSpriteWarp::load(node->get_child(u"effect")));
        for (int i = 0;; i++)
        {
            auto e = "effect" + std::to_string(i);
            if (node->get_child(e))
            {
                effects.push_back(AnimatedSpriteWarp::load(node->get_child(e)));
            }
            else
            {
                break;
            }
        }
    }
    if (auto hit = node->get_child(u"hit"))
    {
        hits.push_back(AnimatedSpriteWarp::load(hit->get_child(u"0")));
    }
    for (int i = 1; i < node->get_child(u"level")->children_count() + 1; i++)
    {
        auto it = node->get_child(u"level")->get_child(std::to_string(i));
        if (it == nullptr)
        {
            break;
        }
        level.push_back(it);
    }
    if (node->get_child(u"action"))
    {
        action_str = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"action")->get_child(u"0"))->get();
    }
    ball = node->get_child(u"ball");
    summon = node->get_child(u"summon");

    node = Wz::Sound->get_root()->find_from_path(u"Skill.img/" + id);
    for (auto &[key, val] : node->get_children())
    {
        auto sou = SoundWarp::load(val[0]);
        sounds[key] = sou;
    }
}

SkillWarp *SkillWarp::load(const std::u16string &id)
{
    if (cache.contains(id))
    {
        return cache[id];
    }
    else
    {
        SkillWarp *ski = new SkillWarp(id);
        cache[id] = ski;
        return ski;
    }
}

Skill::Skill(const std::u16string &id)
{
    skiw = SkillWarp::load(id);
}

void SkillWarp::clean_up()
{
    for (auto &[key, val] : cache)
    {
        delete val;
    }
    cache.clear();
}