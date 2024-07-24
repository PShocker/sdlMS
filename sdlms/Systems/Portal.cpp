module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module systems;

import core;
import components;
import commons;

void portal_run()
{
    if (auto ent = Player::ent; World::registry.valid(ent))
    {
        player_portal(&ent);
    }
}

void player_portal(entt::entity *ent)
{
    auto tr = World::registry.try_get<Transform>(*ent);
    auto view = World::registry.view<Portal>();
    for (auto &e : view)
    {
        auto por = &view.get<Portal>(e);
        if (por->tm != 999999999)
        {
            if (por->pt == 3 || Input::is_key_held(SDLK_UP))
            {
                auto player_pos = tr->position;
                auto por_tr = por->tr;
                auto por_x = por_tr->position.x;
                auto por_y = por_tr->position.y;
                if (player_pos.x == std::clamp(player_pos.x, por_x - 20, por_x + 20) &&
                    player_pos.y == std::clamp(player_pos.y, por_y - 50, por_y + 50))
                {
                    if (por->tm != Map::id)
                    {
                        // need to change map
                        
                    }
                    else
                    {
                        if (Portal::pors.contains(por->tn))
                        {
                            por = Portal::pors[por->tn];
                            tr->position = por->tr->position + SDL_FPoint{0, -5};
                        }
                    }
                    break;
                }
            }
        }
    }
}
