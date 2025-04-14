#pragma once

#include "Components/Components.h"
#include <vector>

struct WorldMap
{
    struct BaseImg
    {
        Sprite spr;
    };

    struct MapLink
    {
        Sprite spr;
        std::u16string linkMap;
    };

    struct CurPos
    {
        std::optional<SDL_FPoint> p;
        AnimatedSprite aspr;
    };

    struct Spot
    {
        SDL_FPoint p;
        std::vector<int> mapNo;
        Sprite spr;
    };
    static void show(std::u16string path = u"");
    static void hide();
    static void click();
    static void over();
    static bool mousein();
    static Spot *mousein_maplist();
    static MapLink *mousein_maplink();
    static void load_baseimg(wz::Node *node);
    static void load_backgrnd();
    static void load_spot(wz::Node *node);
    static void load_link(wz::Node *node);
    static void load_parentmap(wz::Node *node);
    static wz::Node *load_worldmap_node(int map_id);

    static inline BaseImg baseimg;
    static inline CurPos curpos;
    static inline std::vector<Spot> spots;
    static inline std::vector<MapLink> links;
    static inline MapLink *cur_link;
    static inline SDL_Texture *backgrnd;
    static inline std::u16string parentMap;

    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtClose;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtHide2;

    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, SDL_FRect> position_map = {
        {&BtClose, SDL_FRect{-19.0, 6.0, 12.0, 12.0}},
        {&BtHide2, SDL_FRect{-33.0, 6.0, 12.0, 12.0}},
    };

    static void BtClose_func();
    static void BtHide2_func();
    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, std::function<void()>> click_map = {
        {&BtClose, BtClose_func},
        {&BtHide2, BtHide2_func},
    };

    static inline bool open = false;
    static inline float x = 0;
    static inline float y = 0;
};
