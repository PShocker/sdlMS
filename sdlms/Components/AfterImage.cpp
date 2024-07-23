module;

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

module components;

import resources;
import :character;

void AfterImage::init()
{
    auto node = Wz::Character->get_root()->find_from_path(u"Afterimage/swordOS.img");
    for (auto &[first, level] : node->get_children())
    {
        if (first != u"charge")
        {
            for (auto &[second, type] : level[0]->get_children())
            {
                auto action = Character::type_map.at(second);
                int index = 0;
                Animated *a = nullptr;
                SDL_FPoint lt;
                SDL_FPoint rb;
                for (auto &[third, val] : type[0]->get_children())
                {
                    if (third == u"lt")
                    {
                        auto vec = dynamic_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
                        lt = {(float)vec.x, (float)vec.y};
                    }
                    else if (third == u"rb")
                    {
                        auto vec = dynamic_cast<wz::Property<wz::WzVec2D> *>(val[0])->get();
                        lt = {(float)vec.x, (float)vec.y};
                    }
                    else
                    {
                        index = std::stoi(std::string{third.begin(), third.end()});
                        a = new Animated(val[0]);
                    }
                }
                swordOS[first][action] = {index, a, lt, rb};
            }
        }
    }
}


