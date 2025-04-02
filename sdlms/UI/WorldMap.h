#pragma once

#include "Components/Components.h"
#include <vector>

struct WorldMap
{
    struct BaseImg
    {
        Sprite spr;
    };

    struct CurPos
    {
        std::optional<int> x;
        std::optional<int> y;
        AnimatedSprite aspr;
    };

    struct Spot
    {
        int x;
        int y;
        std::vector<int> mapNo;
        Sprite spr;
    };
    static void show();
    static void hide();
    static void click();
    static void over();
    static void run();

    static inline BaseImg baseimg;
    static inline CurPos curpos;
    static inline std::vector<Spot> spots;

    static inline bool open = false;
    static inline bool keyborard = true;
    static inline float x = 0;
    static inline float y = 0;
};
