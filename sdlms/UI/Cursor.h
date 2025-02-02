#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"

struct Cursor
{
    static inline std::unordered_map<std::u16string, std::vector<SDL_Cursor *>> cursor;

    static void init();

    static void load(std::u16string path);

};
