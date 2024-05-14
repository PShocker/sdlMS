#pragma once
#include "World.h"
#include "wz/Property.hpp"
#include "Components/Sound.h"

// 用于加载地图中各种组件
class Map
{
public:
    static void load(int map_id,World *world);
    static void load_obj(wz::Node *node,World *world);
    static void load_tile(wz::Node *node,World *world);
    static void load_background(wz::Node *node,World *world);
    static void load_string(int mapId,World *world);
    static void load_foothold(wz::Node *node,World *world);
    static void load_life(wz::Node *node,World *world);
    static void load_border(wz::Node *node,World *world);
    static void load_ladderRope(wz::Node *node,World *world);
    static void load_portal(wz::Node *node,World *world);

    static void load_bgm(wz::Node *node,World *world);
    static void clean_up(World *world);

private:
    static wz::Node *load_map_node(int mapId,World *world);
};