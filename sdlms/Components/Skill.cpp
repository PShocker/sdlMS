#include "Skill.h"
#include "Sound.h"

#include "wz/Property.hpp"
#include <string>
#include <SDL3/SDL.h>

#include "Resources/Wz.h"

static std::unordered_map<std::u16string, SkillWarp *> cache;

SkillWarp::SkillWarp(const std::u16string &id) : id(id)
{
    node = Wz::Sound->get_root()->find_from_path(u"Skill.img/" + id);
    for (auto &[key, val] : node->get_children())
    {
        auto sou = SoundWarp::load(val[0]);
        sounds[key] = sou;
    }
    node = Wz::Skill->get_root()->find_from_path(id.substr(0, id.length() - 4) + u".img/skill/" + id);
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
    // 加载动作
    if (node->get_child(u"action"))
    {
        if (node->get_child(u"action")->type == wz::Type::String)
        {
            action_str = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"action"))->get();
        }
        else
        {
            // 0,1分别代表单手武器,双手武器的特效,这里就取默认的0
            action_str = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"action/0"))->get();
        }
    }
    // 加载特效
    if (node->get_child(u"effect"))
    {
        if (node->get_child(u"effect")->get_child(u"0")->type == wz::Type::Canvas)
        {
            effects.push_back(AnimatedSpriteWarp::load(node->get_child(u"effect")));
        }
        else
        {
            effects.push_back(AnimatedSpriteWarp::load(node->find_from_path(u"effect/0")));
        }
    }
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