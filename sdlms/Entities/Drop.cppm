module;

#include <string>
#include <stdint.h>

export module entities:drop;

export
{
    void load_drop(const std::u16string &id, uint64_t nums, float x, float y, int z);
};
