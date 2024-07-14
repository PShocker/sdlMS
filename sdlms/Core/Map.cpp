#include "Map.h"
#include "World.h"
#include "../Resource/Wz.h"
#include "../Entities/Tile.h"
#include "../Entities/Obj.h"
#include "../Components/Transform.h"
#include "../Entities/BackGround.h"

void Map::load(int map_id)
{
    auto node = load_map_node(map_id);
    load_tile(node);
    load_obj(node);
    load_background(node);
    sort();
}

void Map::load_obj(wz::Node *node)
{
    auto _node = node;
    for (size_t i = 0; i < 8; i++)
    {
        node = _node->get_child(std::to_string(i));

        for (auto &[key, val] : node->get_child(u"obj")->get_children())
        {
            auto id = std::stoi(std::string{key.begin(), key.end()});
            Obj::load(val[0], id, i);
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
            for (auto &[key, val] : node->get_child(std::to_string(i))->get_child(u"tile")->get_children())
            {
                auto id = std::stoi(std::string{key.begin(), key.end()});
                Tile::load(val[0], dynamic_cast<wz::Property<wz::wzstring> *>(tS)->get(), i, id);
            }
        }
    }
}

void Map::load_background(wz::Node *node)
{
    node = node->get_child(u"back");
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            auto id = std::stoi(std::string{key.begin(), key.end()});
            BackGround::load(val[0], id);
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

void Map::sort()
{
    World::registry.sort<Transform>([](const Transform &m, const Transform &n)
                                    { return m.z < n.z; });
}
