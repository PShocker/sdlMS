#include "Border.h"
#include "FootHold.h"
#include "Components/Line.h"

Border::Border(wz::Node *node, World *world)
{
    if (node->find_from_path(u"info/VRLeft") != nullptr)
    {
        // 优先从mapinfo获取边界
        node = node->find_from_path(u"info");
        left = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRLeft"))->get();
        right = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRRight"))->get();
        top = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRTop"))->get();
        bottom = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRBottom"))->get();
    }
    else
    {
        if (world->entity_exist_of_type<FootHold>())
        {
            // 从fh计算出来
            for (auto &[id, fh] : world->get_entitys<FootHold>())
            {
                auto rl = fh->get_component<Line>();
                auto x1 = rl->get_m().x;
                auto y1 = rl->get_m().y;
                auto x2 = rl->get_n().x;
                auto y2 = rl->get_n().y;
                if (!left.has_value())
                {
                    left = std::min({x1, x2});
                }
                left = std::min({left.value(), x1, x2});
                if (!right.has_value())
                {
                    right = std::max({x1, x2});
                }
                right = std::max({right.value(), x1, x2});
                if (!top.has_value())
                {
                    top = std::min({y1, y2});
                }
                top = std::min({top.value(), y1, y2});
                if (!bottom.has_value())
                {
                    bottom = std::max({y1, y2});
                }
                bottom = std::max({bottom.value(), y1, y2});
            }
            left = left.value() + 25;
            right = right.value() - 25;
            top = top.value() - 300;
            bottom = bottom.value() + 100;
        }
    }
}

Border::~Border()
{
}
