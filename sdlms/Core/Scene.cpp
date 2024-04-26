#include "Scene.h"
#include "Entities/Tile.h"
#include "Resource/Wz.h"

Scene::Scene(World *world)
{
    this->world = world;
}

void Scene::load_map(int mapId)
{
    auto node = load_map_node(mapId);
    load_bgm(node);
    load_tile(node);
}

void Scene::load_tile(wz::Node *node)
{
    auto _node = node;
    for (size_t i = 0; i < 8; i++)
    {
        node = _node->get_child(std::to_string(i));
        auto tS = node->get_child(u"info")->get_child(u"tS");

        if (tS != nullptr)
        {
            for (auto it : node->get_child(u"tile")->get_children())
            {
                Tile *tile = new Tile(it.second[0], dynamic_cast<wz::Property<wz::wzstring> *>(tS)->get(), world);
                world->add_entity(tile);
            }
        }
    }
}

void Scene::load_bgm(wz::Node *node)
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

wz::Node *Scene::load_map_node(int mapId)
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
