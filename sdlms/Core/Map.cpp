module;

#include <string>
#include "wz/Property.hpp"
#include "entt/entt.hpp"

module core;

import resources;
import entities;
import components;

void Map::load(int map_id)
{
    id = map_id;
    auto node = load_map_node(map_id);
    load_tiles(node);
    load_objs(node);
    load_backgrounds(node);
    load_portals(node, map_id);
    load_footholds(node, map_id);
    load_border(node);
    load_ladderRopes(node);
    World::zindex = true;
}

void Map::load_objs(wz::Node *node)
{
    auto _node = node;
    for (size_t i = 0; i < 8; i++)
    {
        node = _node->get_child(std::to_string(i));

        for (auto &[key, val] : node->get_child(u"obj")->get_children())
        {
            auto id = std::stoi(std::string{key.begin(), key.end()});
            load_obj(val[0], id, i);
        }
    }
}

void Map::load_tiles(wz::Node *node)
{
    for (size_t i = 0; i < 8; i++)
    {
        auto tS = node->get_child(std::to_string(i))->get_child(u"info")->get_child(u"tS");

        if (tS != nullptr)
        {
            for (auto &[key, val] : node->get_child(std::to_string(i))->get_child(u"tile")->get_children())
            {
                auto id = std::stoi(std::string{key.begin(), key.end()});
                load_tile(val[0], dynamic_cast<wz::Property<wz::wzstring> *>(tS)->get(), i, id);
            }
        }
    }
}

void Map::load_backgrounds(wz::Node *node)
{
    node = node->get_child(u"back");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            auto id = std::stoi(std::string{key.begin(), key.end()});
            load_background(val[0], id);
        }
    }
}

void Map::load_footholds(wz::Node *node, int map_id)
{
    node = node->get_child(u"foothold");
    if (node != nullptr)
    {
        for (auto &[page, val] : node->get_children())
        {
            for (auto &[zmass, val] : val[0]->get_children())
            {
                for (auto &[key, val] : val[0]->get_children())
                {
                    auto id = std::stoi(std::string{key.begin(), key.end()});
                    load_foothold(val[0], std::stoi(std::string{page.begin(), page.end()}), std::stoi(std::string{zmass.begin(), zmass.end()}), id);
                }
            }
        }
        fix_foothold();
    }
}

void Map::load_border(wz::Node *node)
{
    if (node->find_from_path(u"info/VRLeft") != nullptr)
    {
        // 优先从mapinfo获取边界
        node = node->find_from_path(u"info");
        Border::l = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRLeft"))->get();
        Border::r = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRRight"))->get();
        Border::t = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRTop"))->get();
        Border::b = dynamic_cast<wz::Property<int> *>(node->get_child(u"VRBottom"))->get();
    }
    else
    {
        auto view = World::registry->view<FootHold>();
        for (auto &ent : view)
        {
            auto fh = &view.get<FootHold>(ent);
            auto l = fh->l;
            auto r = fh->r;
            auto t = fh->t;
            auto b = fh->b;

            if (!Border::l.has_value())
            {
                Border::l = l;
            }
            else
            {
                Border::l = std::min(Border::l.value(), l);
            }
            if (!Border::r.has_value())
            {
                Border::r = r;
            }
            else
            {
                Border::r = std::max(Border::r.value(), r);
            }
            if (!Border::t.has_value())
            {
                Border::t = t;
            }
            else
            {
                Border::t = std::min(Border::t.value(), t);
            }
            if (!Border::b.has_value())
            {
                Border::b = b;
            }
            else
            {
                Border::b = std::max(Border::b.value(), b);
            }
        }
        Border::l = Border::l.value() + 25;
        Border::r = Border::r.value() - 25;
        Border::t = Border::t.value() - 300;
        Border::b = Border::b.value() + 100;
    }
}

void Map::load_ladderRopes(wz::Node *node)
{
    node = node->get_child(u"ladderRope");
    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            auto id = std::stoi(std::string{it.first.begin(), it.first.end()});
            load_ladderRope(it.second[0], id);
        }
    }
}

void Map::load_portals(wz::Node *node, int map_id)
{
    node = node->get_child(u"portal");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            auto id = std::stoi(std::string{key.begin(), key.end()});
            load_portal(val[0], id);
        }
        fix_portal();
    }
}

wz::Node *Map::load_map_node(int map_id)
{
    auto node = Wz::Map->get_root();
    auto s = std::to_string(map_id);
    if (s.size() < 9)
    {
        s.insert(0, 9 - s.size(), '0');
    }
    std::string path = "Map/Map" + std::to_string(map_id / 100000000) + "/" + s + ".img";
    return node->find_from_path(path);
}