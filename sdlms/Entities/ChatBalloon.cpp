#include "ChatBalloon.h"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

entt::entity load_chatballoon(std::vector<std::u16string> strs, SDL_Color color, entt::entity owner)
{
    auto ent = World::registry->create();
    auto count = World::registry->view<ChatBalloon>().size();
    World::registry->emplace<Transform>(ent, 0, 0, 8 * LAYER_Z + count);
    auto chatballoon = &World::registry->emplace<ChatBalloon>(ent);
    chatballoon->owner = owner;
    for (auto &str : strs)
    {
        ChatBalloon::push(chatballoon, str, color);
    }
    return ent;
}