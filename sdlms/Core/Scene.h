#pragma once
#include "World.h"
#include "wz/Property.hpp"
#include "Components/Sound.h"

// Scene在这里相当于map,用于加载地图中各种组件
class Scene
{
public:
    Scene(World *world);
    void load_map(int mapId);
    void load_obj(wz::Node *node);
    void load_tile(wz::Node *node);
    void load_bgm(wz::Node *node);

private:
    wz::Node *load_map_node(int mapId);

private:
    World *world;
};