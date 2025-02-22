#pragma once

#include "Components/Components.h"
#include <vector>

struct WorldMap
{
    struct Spot
    {
        int x;
        int y;
        int type;
        std::vector<int> mapNo;

        Spot(int x, int y, int type, std::vector<int> mapNo) : x(x), y(y), type(type), mapNo(mapNo) {}
    };
    static void show();
    static void hide();
    static void click();
    static bool over();
    static void run();

    static inline bool open = false;
    static inline bool keyborard = true;
    static inline float x = 0;
    static inline float y = 0;
};
