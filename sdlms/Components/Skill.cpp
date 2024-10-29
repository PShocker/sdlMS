module;

#include "wz/Property.hpp"
#include <string>
#include <SDL3/SDL.h>

module components;

import :animatedsprite;
import resources;

static std::unordered_map<std::u16string, SkillWarp *> cache;

SkillWarp::Info::Info(wz::Node *node)
{
    if (node->get_child(u"mobCount"))
    {
        mobCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"mobCount"))->get();
    }

    // damage = dynamic_cast<wz::Property<int> *>(node->get_child(u"damage"))->get();
    if (node->get_child(u"attackCount"))
    {
        attackCount = dynamic_cast<wz::Property<int> *>(node->get_child(u"attackCount"))->get();
    }

    if (node->get_child(u"lt") && node->get_child(u"rb"))
    {
        {
            auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"lt"))->get();
            lt = SDL_FPoint{(float)v.x, (float)v.y};
        }
        {
            auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(node->get_child(u"rb"))->get();
            rb = SDL_FPoint{(float)v.x, (float)v.y};
        }
    }
    else if (node->get_child(u"range"))
    {
        auto r = dynamic_cast<wz::Property<int> *>(node->get_child(u"range"))->get();
        lt = SDL_FPoint{(float)-r, (float)-50};
        rb = SDL_FPoint{(float)0, (float)0};
    }
}

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

    auto level = node->get_child(u"level");
    for (int i = 1; i < level->children_count() + 1; i++)
    {
        auto it = level->get_child(std::to_string(i));
        Info *info = new Info(it);
        infos.push_back(info);
    }
    if (node->get_child(u"action"))
    {
        action_str = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"action")->get_child(u"0"))->get();
    }
    if (node->get_child(u"ball"))
    {
        ball = ball->load(node->get_child(u"ball"));
    }

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
    ski = SkillWarp::load(id);
}
