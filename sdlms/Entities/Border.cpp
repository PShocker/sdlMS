#include "Border.h"
#include "FootHold.h"
#include "Components/RigidLine.h"

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
                auto rl = fh->get_component<RigidLine>();
                auto x1 = rl->get_m().x;
                auto y1 = rl->get_m().y;
                auto x2 = rl->get_n().x;
                auto y2 = rl->get_n().y;
                left = std::min({left, x1, x2});
                right = std::max({right, x1, x2});
                top = std::min({top, y1, y2});
                bottom = std::max({bottom, y1, y2});
            }
            left += 25;
            right -= 25;
            top -= 300;
            bottom += 100;
        }
        else
        {
            // 该地图不存在fh
            left = -10000;
            right = 10000;
            top = -10000;
            bottom = 10000;
        }
    }
}

Border::~Border()
{
}
