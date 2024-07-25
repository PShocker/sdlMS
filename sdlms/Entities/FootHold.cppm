module;

#include "wz/Property.hpp"

export module entities:foothold;
import components;

export
{
    void load_foothold(wz::Node * node, int page, int zmass, int id);
    void fix_foothold();
    std::unordered_map<int, FootHold *> fhs;
};
