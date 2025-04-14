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
        std::unordered_map<std::u16string, std::unordered_map<uint8_t, Wrap>> part;
        for (auto &[second, type] : afterimage->get_children())
        {
            if (Character::type_map.contains(second))
            {
                auto action = Character::type_map.at(second);
                Wrap wrap;
                for (auto &[third, act] : type[0]->get_children())
                {
                    if (third == u"lt")
                    {
                        auto vec = dynamic_cast<wz::Property<wz::WzVec2D> *>(act[0])->get();
                        wrap.lt = {(float)vec.x, (float)vec.y};
                    }
                    else if (third == u"rb")
                    {
                        auto vec = dynamic_cast<wz::Property<wz::WzVec2D> *>(act[0])->get();
                        wrap.rb = {(float)vec.x, (float)vec.y};
                    }
                    else
                    {
                        wrap.index = std::stoi(std::string{third.begin(), third.end()});
                        wrap.asprw = AnimatedSprite::Wrap::load(act[0]);
                    }
                }
                part[afterImage_index][action] = wrap;
            }
        }
        afterimages[type] = part;

        if (!hits.contains(type + u"1"))
        {
            auto node = afterimage_node->find_from_path(u"hit.img");
            if (node->find_from_path(type + u"1") == nullptr)
            {
                hits[type + u"1"] = AnimatedSprite::Wrap::load(node->find_from_path(u"sword1"));
                hits[type + u"2"] = AnimatedSprite::Wrap::load(node->find_from_path(u"sword2"));
                hits[type + u"F"] = AnimatedSprite::Wrap::load(node->find_from_path(u"swordF"));
            }
            else
            {
                hits[type + u"1"] = AnimatedSprite::Wrap::load(node->find_from_path(type + u"1"));
                hits[type + u"2"] = AnimatedSprite::Wrap::load(node->find_from_path(type + u"2"));
                hits[type + u"F"] = AnimatedSprite::Wrap::load(node->find_from_path(type + u"F"));
            }
        }
        if (!sounds.contains(sfx))
        {
            auto node = Wz::Sound->get_root()->find_from_path(u"Weapon.img/" + sfx + u"/Attack");
            auto sou = Sound::Wrap::load(node);
            sounds[sfx].push_back(sou);
        }
    }
}
