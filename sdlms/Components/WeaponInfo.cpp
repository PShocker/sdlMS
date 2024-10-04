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

        if (weapon_node->get_child(u"stand1") != nullptr)
        {
            stand1 = true;
        }
        if (weapon_node->get_child(u"walk1") != nullptr)
        {
            walk1 = true;
        }
    }
}
