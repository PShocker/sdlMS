module;
#include "wz/Property.hpp"
module components;

StaticSprite::StaticSprite(wz::Node *node, int alpha)
{
    spr = load_sprite(node, alpha);
}
