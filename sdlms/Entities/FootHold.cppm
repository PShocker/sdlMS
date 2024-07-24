module;

#include "wz/Property.hpp"

export module entities:foothold;

export
{
    void load_foothold(wz::Node * node, int page, int zmass, int id);
    void fix_foothold();
};
