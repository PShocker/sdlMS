#include "Tile.h"
#include "Components/Sprite.h"
#include "Components/Transform.h"

Tile::Tile(wz::Node *node)
{
    Sprite *spr = new Sprite{node};
    Transform *t = new Transform{};
    
    add_component(t);
    add_component(spr);
}