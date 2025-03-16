#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"

struct UIBuff
{
    std::u16string id;

    static void run();

    unsigned int destory;
    unsigned int duration;

    unsigned int index;
};
