#include "WeaponInfo.h"
#include <string>
#include "wz/Property.hpp"
#include "Components/Components.h"

WeaponWrap::WeaponWrap(const std::u16string &id)
{
    auto weapon_node = Character::character_node->find_from_path(u"Weapon/" + id + u".img");
    if (weapon_node != nullptr)
    {
        auto wrap = weapon_node->get_child(u"info");
        afterImage = dynamic_cast<wz::Property<wz::wzstring> *>(wrap->get_child(u"afterImage"))->get();
        sfx = dynamic_cast<wz::Property<wz::wzstring> *>(wrap->get_child(u"sfx"))->get();

        if (wrap->get_child(u"walk") != nullptr)
        {
            if (dynamic_cast<wz::Property<int> *>(wrap->get_child(u"walk"))->get() == 1)
            {
                walk1 = true;
            }
        }
        if (wrap->get_child(u"stand") != nullptr)
        {
            if (dynamic_cast<wz::Property<int> *>(wrap->get_child(u"stand"))->get() == 1)
            {
                stand1 = true;
            }
        }
        if (wrap->get_child(u"stand") != nullptr)
        {
            if (dynamic_cast<wz::Property<int> *>(wrap->get_child(u"stand"))->get() == 1)
            {
                stand1 = true;
            }
        }
        if (wrap->get_child(u"attack") != nullptr)
        {
            attack = dynamic_cast<wz::Property<unsigned short> *>(wrap->get_child(u"attack"))->get();
        }
        if (wrap->get_child(u"reqLevel") != nullptr)
        {
            reqLevel = dynamic_cast<wz::Property<int> *>(wrap->get_child(u"reqLevel"))->get();
        }
        std::string str = std::to_string(reqLevel / 10);
        afterImage_index = std::u16string(str.begin(), str.end());
        AfterImage::load(afterImage, sfx, afterImage_index);
    }
}

bool WeaponWrap::if_long_range_weapon(int attack)
{
    if (attack == WeaponWrap::Attack::BOW ||
        attack == WeaponWrap::Attack::CROSSBOW ||
        attack == WeaponWrap::Attack::CLAW ||
        attack == WeaponWrap::Attack::GUN)
    {
        return true;
    }
    else
    {
        return false;
    }
}
