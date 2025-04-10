#include "World.h"
#include "Map.h"
#include "Window.h"
#include "UI/UI.h"
#include "Components/Components.h"
#include "entt/entt.hpp"

void World::load_map(int id)
{
    // 获取实体存储容器并遍历
    for (auto ent : World::registry->storage<entt::entity>())
    {
        if (ent == Player::ent || World::registry->all_of<Character, Summon>(ent))
        {
            // 自己和自己的影分身
            auto sum = World::registry->try_get<Summon>(ent);
            if (ent == Player::ent || (sum != nullptr && sum->owner == Player::ent))
            {
                World::registry->remove<AfterImage>(ent);
                World::registry->remove<Skill>(ent);
                World::registry->try_get<Effect>(ent)->effects.clear();
                World::registry->try_get<Damage>(ent)->damages.clear();
                continue;
            }
        }
        // 跳过自己的召唤物
        else if (auto sum = World::registry->try_get<Summon>(ent))
        {
            if (sum->owner == Player::ent)
            {
                if (sum->a.contains(u"fly"))
                {
                    sum->state = Summon::State::FLY;
                    sum->index = u"fly";
                }
                else if (sum->a.contains(u"jump"))
                {
                    sum->state = Summon::State::JUMP;
                    sum->index = u"jump";
                }
                sum->a[sum->index].anim_index = 0;
                sum->a[sum->index].anim_time = 0;
                World::registry->try_get<Transform>(ent)->position.x = std::numeric_limits<float>::max();
                continue;
            }
        }
        // 跳过自己的宠物
        else if (auto pet = World::registry->try_get<Pet>(ent))
        {
            World::registry->try_get<Transform>(ent)->position.x = std::numeric_limits<float>::max();
            continue;
        }
        // 跳过右上角UIBuff
        else if (auto uib = World::registry->try_get<UIBuff>(ent))
        {
            continue;
        }
        //
        if (auto nametag = World::registry->try_get<NameTag>(ent))
        {
            for (auto &it : nametag->nametags)
            {
                SDL_DestroyTexture(it.str_texture);
                if (it.back_texture != nullptr)
                {
                    SDL_DestroyTexture(it.back_texture);
                }
            }
        }
        if (auto chatballoon = World::registry->try_get<ChatBalloon>(ent))
        {
            for (auto &it : chatballoon->chatballoons)
            {
                SDL_DestroyTexture(it.str_texture);
                SDL_DestroyTexture(it.back_texture);
            }
        }
        World::registry->destroy(ent);
    }
    WorldMap::hide();
    UIItem::hide();
    KeyConfig::hide();
    UISkill::hide();
    UIEquip::hide();

    Map::load(id);
    MiniMap::show();
    ToolTip::show();

    Window::tick_delta_time();
}
