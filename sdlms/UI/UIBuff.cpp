#include "UIBuff.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"
#include "Commons/Commons.h"

void UIBuff::run()
{
    auto buff = World::registry->try_get<Buff>(Player::ent);
    std::unordered_set<std::u16string> sets;
    std::map<unsigned int, entt::entity> maps;
    for (auto ent : World::registry->view<UIBuff>())
    {
        auto uib = World::registry->try_get<UIBuff>(ent);
        if (buff->buffs.contains(uib->id))
        {
            uib->destory = buff->buffs[uib->id].destory;
            sets.emplace(uib->id);
            maps.emplace(uib->index, ent);
        }
        else
        {
            World::destory.push_back(ent);
            World::zindex = true;
        }
    }
    int i = 0;
    for (auto &[key, val] : maps)
    {
        auto uib = World::registry->try_get<UIBuff>(val);
        auto position = &uib->position;
        position->x = Camera::w - 32 - i * 32;
        uib->index = i;
        i++;
    }

    for (auto &[key, val] : buff->buffs)
    {
        if (!sets.contains(key))
        {
            auto id = key;
            auto ent = World::registry->create();
            auto uib = &World::registry->emplace<UIBuff>(ent);
            uib->id = id;
            uib->destory = val.destory;
            uib->duration = val.duration;
            uib->index = i;
            auto sprw = SpriteWarp::load(
                Wz::Skill->get_root()->find_from_path(id.substr(0, id.length() - 4) + u".img/skill/" + id + u"/icon"));
            World::registry->emplace<Sprite>(ent, sprw);
            uib->position = SDL_FPoint{(float)Camera::w - 32 - i * 32, (float)32};
            World::zindex = true;
            i++;
        }
    }
}
