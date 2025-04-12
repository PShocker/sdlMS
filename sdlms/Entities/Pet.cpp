#include "Pet.h"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

entt::entity load_pet(const u16string id, entt::entity owner)
{
    auto ent = World::registry->create();

    auto node = Wz::Item->get_root()->find_from_path(u"Pet/" + id + u".img");

    auto pet = &World::registry->emplace<Pet>(ent);
    pet->a[u"move"] = AnimatedSprite(node->get_child(u"move"));
    pet->a[u"jump"] = AnimatedSprite(node->get_child(u"jump"));
    pet->a[u"stand0"] = AnimatedSprite(node->get_child(u"stand0"));
    pet->a[u"hang"] = AnimatedSprite(node->get_child(u"hang"));

    for (auto &[key, val] : pet->a)
    {
        for (auto &sprs : val.asprw->sprites)
        {
            SDL_SetTextureScaleMode(sprs->texture, SDL_SCALEMODE_NEAREST);
        }
    }

    pet->index = u"jump";
    pet->id = id;
    pet->owner = owner;

    node = Wz::String->get_root()->find_from_path(u"Pet.img/" + pet->id.substr(pet->id.find_first_not_of(u'0')));
    if (node != nullptr)
    {
        auto name = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"name"))->get();
        auto nametag = &World::registry->emplace<NameTag>(ent);
        NameTag::push(nametag, name, SDL_Color{255, 235, 0, 255});
    }

    World::registry->emplace<Effect>(ent);
    World::registry->emplace<Move>(ent);
    auto count = World::registry->view<Pet>().size();
    auto owner_tr = World::registry->try_get<Transform>(owner);
    World::registry->emplace<Transform>(ent, owner_tr->position, LAYER_Z * 8 + PET_Z - count);
    World::zindex = true;

    return ent;
}
