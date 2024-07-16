module;

#include "wz/Property.hpp"

module entities;

import components;
import resources;
import core;

constexpr const std::u16string pt_list[] = {u"sp", u"pi", u"pv", u"pc", u"pg", u"tp", u"ps",
                                            u"pgi", u"psi", u"pcs", u"ph", u"psh", u"pcj",
                                            u"pci", u"pcig", u"pshg"};

void load_portal(wz::Node *node, int id)
{
    const int z_index = 900000000;

    auto pt = dynamic_cast<wz::Property<int> *>(node->get_child(u"pt"))->get();

    if (pt < 0 || pt >= sizeof(pt_list))
    {
        return;
    }
    else
    {
        auto ent = World::registry.create();

        auto tm = dynamic_cast<wz::Property<int> *>(node->get_child(u"tm"))->get();
        auto pn = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"pn"))->get();
        auto tn = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"tn"))->get();

        auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
        auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

        World::registry.emplace<Transform>(ent, x, y, id + z_index);

        if (pt == 7)
        {
            pt = 2;
        }
        auto url = u"MapHelper.img/portal/game/" + pt_list[pt];
    }
}
