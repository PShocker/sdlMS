module;

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

module components;

import resources;
import :character;

void AfterImage::init()
{
    auto afterimage = Wz::Character->get_root()->find_from_path(u"Afterimage");

    for (auto &[key, val] : afterimage->get_children())
    {
        std::unordered_map<std::u16string, std::unordered_map<uint8_t, Info>> part;
        auto node = afterimage->find_from_path(key);
        for (auto &[first, level] : node->get_children())
        {
            if (first != u"charge")
            {
                for (auto &[second, type] : level[0]->get_children())
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
                        part[first][action] = info;
                    }
                }
            }
        }
        afterimages[key.substr(0, key.length() - 4)] = part;
    }

    auto node = afterimage->find_from_path(u"hit.img");
    for (auto &[key, val] : node->get_children())
    {
        hits[key] = AnimatedSpriteWarp::load(val[0]);
    }

    node = Wz::Sound->get_root()->find_from_path(u"Weapon.img");
    for (auto &[key, val] : node->get_children())
    {
        for (auto &[k, v] : val[0]->get_children())
        {
            auto sou = SoundWarp::load(v[0]);
            sounds[key].push_back(sou);
        }
    }
}
