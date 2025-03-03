#include "Summon.h"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_summon(wz::Node *node, entt::entity ent, float x, float y, int z, const u16string &id, entt::entity owner)
{
    auto sum = &World::registry->emplace<Summon>(ent);
    for (auto &[key, val] : node->get_children())
    {
        sum->a[key] = AnimatedSprite(val[0]);
        for (auto &sprs : sum->a[key].asprw->sprites)
        {
            SDL_SetTextureScaleMode(sprs->texture, SDL_SCALEMODE_NEAREST);
        }
        if (key == u"attack1")
        {
            sum->a[key].asprw->sprites[sum->a[key].asprw->sprites.size() - 1]->delay = 1;
        }
    }

    sum->atk.hit = AnimatedSpriteWarp::load(node->parent->find_from_path(u"attack1/info/hit"));

    sum->index = u"summoned";
    sum->id = id;
    sum->owner = owner;

    World::registry->emplace<Animated>(ent);
    World::registry->emplace<Transform>(ent, x, y, z);
    World::zindex = true;
}