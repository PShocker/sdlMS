module;

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

module components;

import resources;
import :character;

void AfterImage::init()
{
    auto afterimage = Wz::Character->get_root()->find_from_path(u"Afterimage");

    auto load = [&afterimage](const std::u16string &url, std::unordered_map<std::u16string, std::unordered_map<uint8_t, Info>> &part) -> void
    {
        auto node = afterimage->find_from_path(url);
        for (auto &[first, level] : node->get_children())
        {
            if (first != u"charge")
            {
                for (auto &[second, type] : level[0]->get_children())
                {
                    auto action = Character::type_map.at(second);
                    Info info;
                    for (auto &[third, val] : type[0]->get_children())
                    {
                        if (third == u"lt")
                        {
                            auto vec = dynamic_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
                            info.lt = {(float)vec.x, (float)vec.y};
                        }
                        else if (third == u"rb")
                        {
                            auto vec = dynamic_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
                            info.rb = {(float)vec.x, (float)vec.y};
                        }
                        else
                        {
                            info.index = std::stoi(std::string{third.begin(), third.end()});
                            info.asprw = AnimatedSpriteWarp::load(val[0]);
                        }
                    }
                    part[first][action] = info;
                }
            }
        }
    };
    load(u"axe.img", axe);
    load(u"swordOS.img", swordOS);

    auto node=afterimage->find_from_path(u"hit.img");
    for (auto &[key, val] : node->get_children()){
        hits[key]=AnimatedSpriteWarp::load(val[0]);
    }
}
