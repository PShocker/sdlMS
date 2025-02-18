#include "FootHold.h"

#include "wz/Property.hpp"
#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

std::unordered_map<int, FootHold *> fhs;

void load_foothold(wz::Node *node, int page, int zmass, int id)
{
    auto ent = World::registry->create();

    auto &fh = World::registry->emplace<FootHold>(ent);

    fh.id = id;
    fh.page = page;
    fh.zmass = zmass;

    fh.next = reinterpret_cast<FootHold *>(dynamic_cast<wz::Property<int> *>(node->get_child(u"next"))->get());
    fh.prev = reinterpret_cast<FootHold *>(dynamic_cast<wz::Property<int> *>(node->get_child(u"prev"))->get());
    fh.x1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"x1"))->get();
    fh.x2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"x2"))->get();
    fh.y1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y1"))->get();
    fh.y2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y2"))->get();

    if (node->get_child(u"forbidFallDown"))
    {
        fh.forbidFallDown = true;
    }

    fh.l = std::min(fh.x1, fh.x2);
    fh.r = std::max(fh.x1, fh.x2);
    fh.t = std::min(fh.y1, fh.y2);
    fh.b = std::max(fh.y1, fh.y2);

    if (fh.x1 != fh.x2)
    {
        // 斜线
        fh.k = ((float)fh.y2 - (float)fh.y1) / ((float)fh.x2 - (float)fh.x1);
        fh.intercept = fh.y1 - fh.k.value() * fh.x1;
        World::registry->emplace<FloorFootHold>(ent);
    }
    else
    {
        World::registry->emplace<WallFootHold>(ent);
    }
}

void link_foothold()
{
    fhs.clear();

    auto view = World::registry->view<FootHold>();
    for (auto &e : view)
    {
        auto fh = &view.get<FootHold>(e);
        fhs[fh->id] = fh;
    }
    for (auto &e : view)
    {
        auto fh = &view.get<FootHold>(e);
        if (fhs.contains(static_cast<int>(reinterpret_cast<uintptr_t>(fh->prev))))
        {
            fh->prev = fhs[static_cast<int>(reinterpret_cast<uintptr_t>(fh->prev))];
        }
        else
        {
            fh->prev = nullptr;
        }
        if (fhs.contains(static_cast<int>(reinterpret_cast<uintptr_t>(fh->next))))
        {
            fh->next = fhs[static_cast<int>(reinterpret_cast<uintptr_t>(fh->next))];
        }
        else
        {
            fh->next = nullptr;
        }
    }
}
