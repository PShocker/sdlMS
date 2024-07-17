module;

#include "wz/Property.hpp"

export module entities:character;

export
{
    void init_character();
    void load_character(wz::Node *node,int id);
};
