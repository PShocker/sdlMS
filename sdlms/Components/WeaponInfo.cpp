module;

#include <string>
#include "wz/Property.hpp"

module components;

import :character;

WeaponInfo::WeaponInfo(const std::u16string &id)
{
    auto weapon_node = Character::character_node->find_from_path(u"Weapon/" + id + u".img");
    if (weapon_node != nullptr)
    {
        auto info = weapon_node->get_child(u"info");
        afterImage = dynamic_cast<wz::Property<wz::wzstring> *>(info->get_child(u"afterImage"))->get();
        sfx = dynamic_cast<wz::Property<wz::wzstring> *>(info->get_child(u"sfx"))->get();

        if (info->get_child(u"walk") != nullptr)
        {
            if (dynamic_cast<wz::Property<int> *>(info->get_child(u"walk"))->get() == 1)
            {
                walk1 = true;
            }
        }
        if (info->get_child(u"stand") != nullptr)
        {
            if (dynamic_cast<wz::Property<int> *>(info->get_child(u"stand"))->get() == 1)
            {
                stand1 = true;
            }
        }
        if (info->get_child(u"stand") != nullptr)
        {
            if (dynamic_cast<wz::Property<int> *>(info->get_child(u"stand"))->get() == 1)
            {
                stand1 = true;
            }
        }
        if (info->get_child(u"attack") != nullptr)
        {
            attack = dynamic_cast<wz::Property<unsigned short> *>(info->get_child(u"attack"))->get();
        }
    }
}
