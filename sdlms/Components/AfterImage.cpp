#include "AfterImage.h"
#include "Character.h"

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

#include "Resources/Wz.h"

std::u16string AfterImage::afterImage_index(int level)
{
    auto level_str = std::to_string(level / 10);
    return std::u16string{level_str.begin(), level_str.end()};
}

void AfterImage::load(std::u16string &type, std::u16string &sfx, int level)
{
    auto u16_level_str = afterImage_index(level);

    auto afterimage_node = Wz::Character->get_root()->find_from_path(u"Afterimage");

    if (!(afterimages.contains(type) && afterimages[type].contains(u16_level_str)))
    {
        auto afterimage = afterimage_node->find_from_path(type + u".img/" + u16_level_str);

        std::unordered_map<std::u16string, std::unordered_map<uint8_t, Info>> part;

        for (auto &[second, type] : afterimage->get_children())
        {
            if (Character::type_map.contains(second))
            {
                auto action = Character::type_map.at(second);
                Info info;
                for (auto &[third, act] : type[0]->get_children())
                {
                    if (third == u"lt")
                    {
                        auto vec = dynamic_cast<wz::Property<wz::WzVec2D> *>(act[0])->get();
                        info.lt = {(float)vec.x, (float)vec.y};
                    }
                    else if (third == u"rb")
                    {
                        auto vec = dynamic_cast<wz::Property<wz::WzVec2D> *>(act[0])->get();
                        info.rb = {(float)vec.x, (float)vec.y};
                    }
                    else
                    {
                        info.index = std::stoi(std::string{third.begin(), third.end()});
                        info.asprw = AnimatedSpriteWarp::load(act[0]);
                    }
                }
                part[u16_level_str][action] = info;
            }
        }
        afterimages[type] = part;

        if (!hits.contains(type + u"1"))
        {
            auto node = afterimage_node->find_from_path(u"hit.img");
            if (node->find_from_path(type + u"1") == nullptr)
            {
                hits[type + u"1"] = AnimatedSpriteWarp::load(node->find_from_path(u"sword1"));
                hits[type + u"2"] = AnimatedSpriteWarp::load(node->find_from_path(u"sword2"));
                hits[type + u"F"] = AnimatedSpriteWarp::load(node->find_from_path(u"swordF"));
            }
            else
            {
                hits[type + u"1"] = AnimatedSpriteWarp::load(node->find_from_path(type + u"1"));
                hits[type + u"2"] = AnimatedSpriteWarp::load(node->find_from_path(type + u"2"));
                hits[type + u"F"] = AnimatedSpriteWarp::load(node->find_from_path(type + u"F"));
            }
        }
        if (!sounds.contains(sfx))
        {
            auto node = Wz::Sound->get_root()->find_from_path(u"Weapon.img/" + sfx + u"/Attack");
            auto sou = SoundWarp::load(node);
            sounds[sfx].push_back(sou);
        }
    }
}
