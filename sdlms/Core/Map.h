#pragma once

#include "entt/entt.hpp"
#include "wz/Property.hpp"
#include <vector>
#include "Components/Components.h"

// 用于加载地图中各种组件
struct Map
{
    static void load(int map_id);
    static void load_objs(wz::Node *node);
    static void load_tiles(wz::Node *node);
    static void load_backgrounds(wz::Node *node);
    static void load_footholds(wz::Node *node, int map_id);
    static void load_lifes(wz::Node *node);
    static void load_borders(wz::Node *node);
    static void load_ladderRopes(wz::Node *node);
    static void load_portals(wz::Node *node, int map_id);
    static void load_reactors(wz::Node *node);

    static void load_bgm(wz::Node *node);
    static void load_bgm(int map_id);

    static std::u16string load_streetname(int map_id);
    static std::u16string load_mapname(int map_id);
    
    static uint32_t load_returnmap(int map_id);

    static inline uint32_t id = 0;

    static wz::Node *load_map_node(int map_id);
    static wz::Node *load_string_node(int map_id);
};
