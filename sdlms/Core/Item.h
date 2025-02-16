#pragma once

#include <string>
#include "wz/Property.hpp"
#include "Components/Components.h"

struct Item
{
    static wz::Node *load(std::u16string item_id);
};
