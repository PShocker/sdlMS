#pragma once

#include "wz/Property.hpp"

// 用于加载地图中各种组件
struct Map
{
    static void load(int map_id);
    static void load_obj(wz::Node *node);
    static void load_tile(wz::Node *node);
    static void load_background(wz::Node *node);
    static void load_string(int map_id);
    static void load_foothold(wz::Node *node);
    static void load_life(wz::Node *node);
    static void load_border(wz::Node *node);
    static void load_ladderRope(wz::Node *node);
    static void load_portal(wz::Node *node);

    static void load_bgm(wz::Node *node);
    static void clean_up();

    static inline uint32_t map_id = 0;

    static wz::Node *load_map_node(int map_id);
    static void sort();
};