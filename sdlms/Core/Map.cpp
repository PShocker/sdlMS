#include "Map.h"
#include "Entities/Tile.h"
#include "Entities/Obj.h"
#include "Entities/BackGround.h"
#include "Entities/FootHold.h"
#include "Entities/Npc.h"
#include "Entities/Border.h"
#include "Entities/LadderRope.h"
#include "Entities/Portal.h"
#include "Resource/Wz.h"
#include "Components/RigidLine.h"
#include "Components/Transform.h"

void Map::load(int map_id, World *world)
{

    clean_up(world);
    auto node = load_map_node(map_id, world);
    load_tile(node, world);
    load_obj(node, world);
    load_background(node, world);
    load_foothold(node, world);
    load_life(node, world);
    load_border(node, world);
    load_ladderRope(node, world);
    load_portal(node, world);
    load_bgm(node, world);
    // load_string(mapId);
}

void Map::load_obj(wz::Node *node, World *world)
{
    auto _node = node;
    for (size_t i = 0; i < 8; i++)
    {
        node = _node->get_child(std::to_string(i));

        for (auto &[key, val] : node->get_child(u"obj")->get_children())
        {
            Obj *obj = new Obj(val[0], std::stoi(std::string{key.begin(), key.end()}), i, world);
            world->add_entity(obj);
        }
    }
}

void Map::load_tile(wz::Node *node, World *world)
{
    for (size_t i = 0; i < 8; i++)
    {
        auto tS = node->get_child(std::to_string(i))->get_child(u"info")->get_child(u"tS");

        if (tS != nullptr)
        {
            for (auto &[key, val] : node->get_child(std::to_string(i))->get_child(u"tile")->get_children())
            {
                Tile *tile = new Tile(val[0], dynamic_cast<wz::Property<wz::wzstring> *>(tS)->get(), i, world);
                world->add_entity(tile);
            }
        }
    }
}

void Map::load_background(wz::Node *node, World *world)
{
    node = node->get_child(u"back");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            BackGround *bac = new BackGround(val[0], std::stoi(std::string{key.begin(), key.end()}), world);
            world->add_entity(bac);
        }
    }
}

void Map::load_string(int mapId, World *world)
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

void Map::load_foothold(wz::Node *node, World *world)
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
                    FootHold *f = new FootHold(val[0], id, std::stoi(std::string{page.begin(), page.end()}), std::stoi(std::string{zmass.begin(), zmass.end()}), world);
                    world->add_entity(f, id);
                }
            }
        }
    }
}

void Map::load_life(wz::Node *node, World *world)
{
    node = node->get_child(u"life");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            auto type = dynamic_cast<wz::Property<wz::wzstring> *>(val[0]->get_child(u"type"))->get();
            if (type == u"n")
            {
                auto npc = new Npc(val[0], world);
                world->add_entity(npc);
            }
        }
    }
}

void Map::load_border(wz::Node *node, World *world)
{
    auto border = new Border(node, world);
    world->add_entity(border);
}

void Map::load_ladderRope(wz::Node *node, World *world)
{
    node = node->get_child(u"ladderRope");
    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            auto id = std::stoi(std::string{it.first.begin(), it.first.end()});
            auto lad = new LadderRope(it.second[0], id, world);
            world->add_entity(lad, id);
        }
    }
}

void Map::load_portal(wz::Node *node, World *world)
{
    node = node->get_child(u"portal");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            Portal *por = new Portal(val[0], world);
            world->add_entity(por);
        }
    }
}

void Map::load_bgm(wz::Node *node, World *world)
{
    node = node->find_from_path("info/bgm");
    if (node != nullptr)
    {
        auto url = dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
        url.insert(url.find('/'), u".img");
        node = world->get_resource<Wz>().Sound->get_root()->find_from_path(url);
        auto sou = Sound::load(node);
        world->add_unique_component(sou);
    }
    return;
}

void Map::clean_up(World *world)
{
    world->clear_entity<Tile>();
    world->clear_entity<Obj>();
    world->clear_entity<BackGround>();
    world->clear_entity<FootHold>();
    world->clear_entity<Npc>();
    world->clear_entity<Border>();
    world->clear_entity<LadderRope>();
    world->clear_entity<Portal>();
}

wz::Node *Map::load_map_node(int mapId, World *world)
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
