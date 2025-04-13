#include "Map.h"
#include "World.h"

#include <string>
#include <unordered_set>
#include "wz/Property.hpp"
#include "entt/entt.hpp"

#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include "Entities/Entities.h"

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
    load_lifes(node);
    load_reactors(node);
    load_bgm(node);
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
        link_foothold();
    }
}

void Map::load_lifes(wz::Node *node)
{
    node = node->get_child(u"life");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            auto type = dynamic_cast<wz::Property<wz::wzstring> *>(val[0]->get_child(u"type"))->get();
            if (type == u"n")
            {
                load_npc(val[0]);
            }
            else if (type == u"m")
            {
                load_mob(val[0]);
            }
        }
    }
}

void Map::load_borders(wz::Node *node)
{
    load_border(node);
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

void Map::load_reactors(wz::Node *node)
{
    node = node->get_child(u"reactor");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            load_reactor(val[0]);
        }
    }
}

void Map::load_bgm(wz::Node *node)
{
    node = node->find_from_path("info/bgm");
    if (node != nullptr)
    {
        auto url = dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
        url.insert(url.find('/'), u".img");
        node = Wz::Sound->get_root()->find_from_path(url);

        Sound sou(node);
        sou.circulate = true;
        sou.bgm = true;
        if (auto first_sound = Sound::at(0))
        {
            if (first_sound->souw != sou.souw)
            {
                if (first_sound->bgm == true)
                {
                    // 移除上一个图的bgm
                    Sound::remove(0);
                    // 切换到本地图的bgm
                    Sound::push(sou, 0);
                }
            }
        }
        else
        {
            Sound::push(sou, 0);
        }
    }
    return;
}

void Map::load_bgm(int map_id)
{
    auto node = load_map_node(map_id);
    load_bgm(node);
}

std::u16string Map::load_streetname(int map_id)
{
    auto node = Map::load_string_node(map_id);
    return dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"streetName"))->get();
}

std::u16string Map::load_mapname(int map_id)
{
    auto node = Map::load_string_node(map_id);
    return dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"mapName"))->get();
}

uint32_t Map::load_returnmap(int map_id)
{
    auto node = load_map_node(map_id);
    node = node->find_from_path(u"info/returnMap");
    if (node != nullptr)
    {
        return dynamic_cast<wz::Property<int> *>(node)->get();
    }
    return 0;
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

wz::Node *Map::load_string_node(int map_id)
{
    std::u16string path;
    if (700000000 <= map_id && map_id <= 782000001)
    {
        path = u"chinese";
    }
    else if (674030000 <= map_id && map_id <= 674030300 || 900000000 <= map_id && map_id <= 999999998)
    {
        path = u"etc";
    }
    else if (0 <= map_id && map_id <= 2000001)
    {
        path = u"maple";
    }
    else if (200000000 <= map_id && map_id <= 280090000)
    {
        path = u"ossyria";
    }
    else if (300000000 <= map_id && map_id <= 300030100)
    {
        path = u"elin";
    }
    else if (500000000 <= map_id && map_id <= 500020400)
    {
        path = u"thai";
    }
    else if (100000000 <= map_id && map_id <= 199000000)
    {
        path = u"victoria";
    }
    else if (680100000 <= map_id && map_id <= 682000901)
    {
        path = u"HalloweenGL";
    }
    else if (800000000 <= map_id && map_id <= 801030000)
    {
        path = u"jp";
    }
    else if (600000000 <= map_id && map_id <= 600020600)
    {
        path = u"MasteriaGL";
    }
    else if (540000000 <= map_id && map_id <= 551030002)
    {
        path = u"SG";
    }
    else if (677000000 <= map_id && map_id <= 677000013)
    {
        path = u"Episode1GL";
    }
    auto s = std::to_string(map_id);
    return Wz::String->get_root()->find_from_path(u"Map.img/" + path + u"/" + std::u16string{s.begin(), s.end()});
}
