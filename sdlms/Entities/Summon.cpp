#include "Summon.h"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

entt::entity load_summon(wz::Node *node, int id, entt::entity owner)
{
    auto ent = World::registry->create();

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
    if (node->find_from_path(u"move/0"))
    {
        sum->a[u"jump"] = AnimatedSprite(node->find_from_path(u"move/0"));
    }

    sum->atk.hit = AnimatedSprite::Wrap::load(node->parent->find_from_path(u"hit/0"));

    sum->index = u"summoned";
    sum->id = id;
    sum->owner = owner;

    World::registry->emplace<Effect>(ent);
    World::registry->emplace<Move>(ent);

    auto owner_tr = World::registry->try_get<Transform>(owner);
    auto owner_mv = World::registry->try_get<Move>(owner);
    auto count = World::registry->view<Summon>().size();
    World::registry->emplace<Transform>(ent, owner_tr->position, owner_mv->foo->page * LAYER_Z + SUMMON_Z + count);
    World::zindex = true;
    return ent;
}