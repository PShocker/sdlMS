module;

#include "wz/Property.hpp"

export module entities:portal;

export
{
    void load_portal(wz::Node * node, int id);
    void fix_portal();
};