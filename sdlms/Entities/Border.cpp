#include "Border.h"

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
        // 从fh计算出来
        node = node->get_child(u"foothold");
        if (node != nullptr)
        {
            int Left, Right, Top, Bottom;
            for (auto it : node->get_children())
            {
                for (auto _it : it.second[0]->get_children())
                {
                    for (auto __it : _it.second[0]->get_children())
                    {
                        auto foothold = __it.second[0];
                        auto x1 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"x1"))->get();
                        auto x2 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"x2"))->get();
                        auto y1 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"y1"))->get();
                        auto y2 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"y2"))->get();
                        left = std::min({left, x1, x2}) + 25;
                        right = std::max({right, x1, x2}) - 25;
                        top = std::min({top, y1, y2}) - 300;
                        bottom = std::max({bottom, y1, y2}) + 100;
                    }
                }
            }
        }
    }
}