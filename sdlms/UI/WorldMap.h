#pragma once

#include "Components/Components.h"
#include <vector>

struct WorldMap
{
    struct BaseImg
    {
    };

    struct CurPos
    {
    };

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

    SDL_FPoint position;

    static inline bool open = false;
    static inline bool keyborard = true;
    static inline float x = 0;
    static inline float y = 0;
};
