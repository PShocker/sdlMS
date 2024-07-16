module;

#include "wz/Property.hpp"

export module entities:tile;

export
{
    void load_tile(wz::Node * node, const std::u16string &ts, int layer, int id);
};
