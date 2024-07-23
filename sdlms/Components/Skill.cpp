module;

#include "wz/Property.hpp"
#include <string>

module components;

import :animatedsprite;
import resources;

void Skill::init()
{
    std::u16string id = u"1311001";
    auto node = Wz::Skill->get_root()->find_from_path(u"131.img/skill/" + id);
    if (node->get_child(u"effect") == nullptr)
    {
        return;
    }
    else
    {
        skill_map[id].push_back(new Animated(node->get_child(u"effect")));
        for (int i = 0;; i++)
        {
            auto e = "effect" + std::to_string(i);
            if (node->get_child(e))
            {
                skill_map[id].push_back(new Animated(node->get_child(e)));
            }
            else
            {
                break;
            }
        }
    }
}

Skill::Skill(const std::u16string &id) : id(id)
{
    animated.resize(skill_map[id].size(), false);
}
