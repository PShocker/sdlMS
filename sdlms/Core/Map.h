#pragma once
#include "World.h"
#include "wz/Property.hpp"
#include "Components/Sound.h"

// 用于加载地图中各种组件
class Map
{
public:
    Map(World *world);
    void load_map(int mapId);
    void load_obj(wz::Node *node);
    void load_tile(wz::Node *node);
    void load_background(wz::Node *node);
    void load_string(int mapId);
    void load_foothold(wz::Node *node);
    void load_life(wz::Node *node);
    void load_border(wz::Node *node);
    void load_ladderRope(wz::Node *node);
    void load_portal(wz::Node *node);

    void load_bgm(wz::Node *node);

private:
    wz::Node *load_map_node(int mapId);
    World *world;
};