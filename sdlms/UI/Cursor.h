#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"

struct Cursor
{
    struct Info
    {
        SDL_Cursor *cursor;
        int delay;
    };

    static inline std::unordered_map<std::u16string, std::vector<Info>> cursor;

    static void init();

    static void load(const std::u16string& path);

    static void run();

    static void action(const std::u16string& t);

    static inline std::u16string type;
    static inline int time = 0;
    static inline int index = 0;
};
