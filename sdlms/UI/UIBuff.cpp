#include "UIBuff.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"
#include "Commons/Commons.h"

void UIBuff::init()
{
    auto node = Wz::UI->get_root()->find_from_path(u"Basic.img/ItemNo");
    number_cache[0] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"0")));
    number_cache[1] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"1")));
    number_cache[2] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"2")));
    number_cache[3] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"3")));
    number_cache[4] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"4")));
    number_cache[5] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"5")));
    number_cache[6] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"6")));
    number_cache[7] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"7")));
    number_cache[8] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"8")));
    number_cache[9] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"9")));
}

void UIBuff::run()
{
    auto buff = World::registry->try_get<Buff>(Player::ent);
    std::unordered_set<int> sets;
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
            auto sprw = Sprite::Wrap::load(Skill::Wrap::load(id)->node->find_from_path(u"icon"));
            World::registry->emplace<Sprite>(ent, sprw);
            uib->position = SDL_FPoint{(float)Camera::w - 32 - i * 32, (float)32};
            i++;
        }
    }
}
