#include "AfterImage.h"
#include "Character.h"

#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"

void AfterImage::load(const std::u16string &type, const std::u16string &sfx, const std::u16string &afterImage_index)
{
    auto afterimage_node = Wz::Character->get_root()->find_from_path(u"Afterimage");
    if (!(afterimages.contains(type) && afterimages[type].contains(afterImage_index)))
    {
        auto afterimage = afterimage_node->find_from_path(type + u".img/" + afterImage_index);
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
                        info.asprw = AnimatedSpriteWarp::load(act[0], 255);
                    }
                }
                part[afterImage_index][action] = info;
            }
        }
        afterimages[type] = part;

        if (!hits.contains(type + u"1"))
        {
            auto node = afterimage_node->find_from_path(u"hit.img");
            if (node->find_from_path(type + u"1") == nullptr)
            {
                hits[type + u"1"] = AnimatedSpriteWarp::load(node->find_from_path(u"sword1"), 255);
                hits[type + u"2"] = AnimatedSpriteWarp::load(node->find_from_path(u"sword2"), 255);
                hits[type + u"F"] = AnimatedSpriteWarp::load(node->find_from_path(u"swordF"), 255);
            }
            else
            {
                hits[type + u"1"] = AnimatedSpriteWarp::load(node->find_from_path(type + u"1"), 255);
                hits[type + u"2"] = AnimatedSpriteWarp::load(node->find_from_path(type + u"2"), 255);
                hits[type + u"F"] = AnimatedSpriteWarp::load(node->find_from_path(type + u"F"), 255);
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
