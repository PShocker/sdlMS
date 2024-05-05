#include "Map.h"
#include "Entities/Tile.h"
#include "Entities/Obj.h"
#include "Entities/BackGround.h"
#include "Entities/FootHold.h"
#include "Entities/Npc.h"
#include "Entities/Border.h"
#include "Resource/Wz.h"

Map::Map(World *world)
{
    this->world = world;
}

void Map::load_map(int mapId)
{
    auto node = load_map_node(mapId);
    load_bgm(node);
    load_tile(node);
    load_obj(node);
    load_background(node);
    load_foothold(node);
    load_life(node);
    load_border(node);
    // load_string(mapId);
}

void Map::load_obj(wz::Node *node)
{
    auto _node = node;
    for (size_t i = 0; i < 8; i++)
    {
        node = _node->get_child(std::to_string(i));

        for (auto it : node->get_child(u"obj")->get_children())
        {
            Obj *obj = new Obj(it.second[0], std::stoi(std::string{it.first.begin(), it.first.end()}), i, world);
            // world->add_entity(obj);
        }
    }
}

void Map::load_tile(wz::Node *node)
{
    for (size_t i = 0; i < 8; i++)
    {
        auto tS = node->get_child(std::to_string(i))->get_child(u"info")->get_child(u"tS");

        if (tS != nullptr)
        {
            for (auto it : node->get_child(std::to_string(i))->get_child(u"tile")->get_children())
            {
                Tile *tile = new Tile(it.second[0], dynamic_cast<wz::Property<wz::wzstring> *>(tS)->get(), i, world);
                // world->add_entity(tile);
            }
        }
    }
}

void Map::load_background(wz::Node *node)
{
    node = node->get_child(u"back");
    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            BackGround *bac = new BackGround(it.second[0], std::stoi(std::string{it.first.begin(), it.first.end()}), world);
        }
    }
}

void Map::load_string(int mapId)
{
    auto node = world->get_resource<Wz>().String->get_root()->find_from_path("Map.img");
    for (auto &[key, val] : node->get_children())
    {
        if (val[0]->get_child(std::to_string(mapId)) != nullptr)
        {

            printf("");
        }
    }
}

void Map::load_foothold(wz::Node *node)
{
    node = node->get_child(u"foothold");
    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            auto page = std::stoi(std::string{it.first.begin(), it.first.end()});
            for (auto _it : it.second[0]->get_children())
            {
                auto zmass = std::stoi(std::string{_it.first.begin(), _it.first.end()});
                for (auto __it : _it.second[0]->get_children())
                {
                    FootHold *f = new FootHold(__it.second[0], std::stoi(std::string{__it.first.begin(), __it.first.end()}), page, zmass, world);
                }
            }
        }
    }
}

void Map::load_life(wz::Node *node)
{
    node = node->get_child(u"life");
    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            auto type = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"type"))->get();

            if (type == u"n")
            {
                auto npc = new Npc(it.second[0], world);
                world->add_entity(npc);
            }
        }
    }
}

void Map::load_border(wz::Node *node)
{
    auto border = new Border(node, world);
    world->add_entity(border);
}

void Map::load_bgm(wz::Node *node)
{
    node = node->find_from_path("info/bgm");
    if (node != nullptr)
    {
        auto url = dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
        url.insert(url.find('/'), u".img");
        auto wz = world->get_resource<Wz>();
        auto node = wz.Sound->get_root();
        node = node->find_from_path(url);
        world->add_component(Sound::load_sound(node));
    }
    return;
}

wz::Node *Map::load_map_node(int mapId)
{
    auto node = world->get_resource<Wz>().Map->get_root();
    auto s = std::to_string(mapId);
    if (s.size() < 9)
    {
        s.insert(0, 9 - s.size(), '0');
    }
    std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + s + ".img";
    return node->find_from_path(path);
}
