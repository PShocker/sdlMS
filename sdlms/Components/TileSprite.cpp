module;
#include "wz/Property.hpp"
module components;

TileSprite::TileSprite(wz::Node *node, int alpha)
{
    spr = load_sprite(node, alpha);
}
