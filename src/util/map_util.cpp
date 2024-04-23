#include "util/map_util.hpp"
#include "util/wz_util.hpp"
#include "sdlms/graphics.hpp"
#include <SDL2/SDL.h>
#include <string>
#include "map_util.hpp"

namespace util
{
    MapUtil::MapUtil()
    {
    }

    std::tuple<int, int, int, int> MapUtil::load_border(int mapId)
    {
        return load_border(load_map_node(mapId));
    }

    std::tuple<int, int, int, int> MapUtil::load_border(wz::Node *node)
    {
        if (node->find_from_path(u"info/VRLeft") != nullptr)
        {
            // 优先从mapinfo获取边界
            node = node->find_from_path(u"info");
            auto VRLeft = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRLeft"))->get();
            auto VRRight = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRRight"))->get();
            auto VRTop = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRTop"))->get();
            auto VRBottom = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRBottom"))->get();
            return {VRLeft, VRRight, VRTop, VRBottom};
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
                            Left = std::min({Left, x1, x2});
                            Right = std::max({Right, x1, x2});
                            Top = std::min({Top, y1, y2});
                            Bottom = std::max({Bottom, y1, y2});
                        }
                    }
                }
                return {Left + 25, Right - 25, Top - 300, Bottom + 100};
            }
        }
        return std::tuple<int, int, int, int>();
    }

    wz::Node *MapUtil::load_map_node(int mapId)
    {
        auto _map_node = WzUtil::current()->Map->get_root();
        auto s = std::to_string(mapId);
        if (s.size() < 9)
        {
            s.insert(0, 9 - s.size(), '0');
        }
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + s + ".img";
        return _map_node->find_from_path(path);
    }

}
