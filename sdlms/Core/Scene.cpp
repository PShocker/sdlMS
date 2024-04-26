#include "Scene.h"
#include "Resource/Wz.h"


Scene::Scene(World *world)
{
    this->world = world;
}

void Scene::load_map(int mapId)
{
    auto node = load_map_node(mapId);
    auto sound = load_bgm(node);
}

Sound *Scene::load_bgm(wz::Node *node)
{
    node = node->find_from_path("info/bgm");
    if (node != nullptr)
    {
        auto url = dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
        url.insert(url.find('/'), u".img");
        node = world->get_resource<Wz>().Sound->get_root()->find_from_path(url);
        return Sound::load_sound(node);
    }
    return nullptr;
}

wz::Node *Scene::load_map_node(int mapId)
{
    auto wz = world->get_resource<Wz>();
    auto node = wz.Map->get_root();
    auto s = std::to_string(mapId);
    if (s.size() < 9)
    {
        s.insert(0, 9 - s.size(), '0');
    }
    std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + s + ".img";
    return node->find_from_path(path);
}
