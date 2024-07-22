module;

#include "wz/Property.hpp"

export module core:map;

// 用于加载地图中各种组件
export struct Map
{
    static void load(int map_id);
    static void load_objs(wz::Node *node);
    static void load_tiles(wz::Node *node);
    static void load_backgrounds(wz::Node *node);
    static void load_string(int map_id);
    static void load_footholds(wz::Node *node);
    static void load_life(wz::Node *node);
    static void load_border(wz::Node *node);
    static void load_ladderRopes(wz::Node *node);
    static void load_portals(wz::Node *node);

    static void load_bgm(wz::Node *node);
    static void clean_up();

    static inline uint32_t map_id = 0;

    struct Border
    {
        static inline optional<int> l = std::nullopt;
        static inline optional<int> r = std::nullopt;
        static inline optional<int> t = std::nullopt;
        static inline optional<int> b = std::nullopt;
    };
    static wz::Node *load_map_node(int map_id);
};
