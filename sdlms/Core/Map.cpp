module;

#include <string>
#include "wz/Property.hpp"

module core;

import resources;
import entities;

void Map::load(int map_id)
{
    auto node = load_map_node(map_id);
    load_tiles(node);
    load_objs(node);
    load_backgrounds(node);
    load_portals(node);
    load_footholds(node);

    World::zindex();
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

void Map::load_footholds(wz::Node *node)
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
    }
}

void Map::load_portals(wz::Node *node)
{
    node = node->get_child(u"portal");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            auto id = std::stoi(std::string{key.begin(), key.end()});
            load_portal(val[0], id);
        }
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