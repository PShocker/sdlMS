#include "Border.h"

#include "wz/Property.hpp"
#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_border(wz::Node *node)
{
    std::optional<int> l = std::nullopt;
    std::optional<int> r = std::nullopt;
    std::optional<int> t = std::nullopt;
    std::optional<int> b = std::nullopt;

    if (node->find_from_path(u"info/VRLeft") != nullptr)
    {
        // 优先从mapinfo获取边界
        node = node->find_from_path(u"info");
        l = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRLeft"))->get();
        r = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRRight"))->get();
        t = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRTop"))->get();
        b = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRBottom"))->get();
    }
    else
    {
        auto view = World::registry->view<FootHold>();
        for (auto ent : view)
        {
            auto fh = &view.get<FootHold>(ent);
            if (!l.has_value())
            {
                l = fh->l;
            }
            else
            {
                l = std::min(l.value(), fh->l);
            }
            if (!r.has_value())
            {
                r = fh->r;
            }
            else
            {
                r = std::max(r.value(), fh->r);
            }
            if (!t.has_value())
            {
                t = fh->t;
            }
            else
            {
                t = std::min(t.value(), fh->t);
            }
            if (!b.has_value())
            {
                b = fh->b;
            }
            else
            {
                b = std::max(b.value(), fh->b);
            }
        }
        l = l.value() + 25;
        r = r.value() - 25;
        t = t.value() - 300;
        b = b.value() + 100;
    }
    World::registry->ctx().erase<Border>();
    World::registry->ctx().emplace<Border>(l, r, t, b);
}