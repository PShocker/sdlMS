module;

#include "wz/Property.hpp"

module components;

import :sprite;
import resources;

void Damage::init()
{
    auto node = Wz::Effect->get_root()->find_from_path(u"BasicEff.img/NoRed1");
    for (int i = 0; i < 10; i++)
    {
        sprs[i]=SpriteWarp::load(node->get_child(std::to_string(i)));
    }
}
