#pragma once

#include "wz/Property.hpp"
#include "Components/Components.h"

void load_foothold(wz::Node *node, int page, int zmass, int id);
void link_foothold();
extern std::unordered_map<int, FootHold *> fhs;
