module;

#include <string>
#include <stdint.h>
#include "entt/entt.hpp"

export module entities:drop;

export
{
    entt::entity load_drop(const std::u16string &id, uint64_t nums, float x, float y, int layer);
};
