#include "Summon.h"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_summon(wz::Node *node, entt::entity ent, float x, float y, int z)
{
    auto sum = &World::registry->emplace<Summon>(ent);
    for (auto &[key, val] : node->get_children())
    {
        sum->a[key] = AnimatedSprite(val[0]);
        for (auto &sprs : sum->a[key].aspr->sprites)
        {
            SDL_SetTextureScaleMode(sprs->texture, SDL_SCALEMODE_NEAREST);
        }
    }
    sum->atk = AnimatedSpriteWarp::load(node->parent->find_from_path(u"hit/0"));
    sum->index = u"summoned";

    World::registry->emplace<Animated>(ent);
    World::registry->emplace<Transform>(ent, x, y, z);
    World::zindex = true;
}