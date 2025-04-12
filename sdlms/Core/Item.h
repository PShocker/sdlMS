#pragma once

#include <string>
#include "wz/Property.hpp"
#include "Components/Components.h"

struct Item
{
    static wz::Node *load(std::u16string item_id);
    static uint8_t load_item_index(std::u16string item_id);
    static wz::Node *load_item_string(std::u16string item_id);
    static std::u16string load_item_name(std::u16string item_id);
};
