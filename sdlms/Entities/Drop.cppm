module;

#include <string>
#include <stdint.h>
#include "entt/entt.hpp"

export module entities:drop;

export
{
    struct DropInfo
    {
        std::u16string id;
        unsigned int nums;
    };

    void load_drops(std::vector<DropInfo> * drops, float x, float y, int layer);
};

