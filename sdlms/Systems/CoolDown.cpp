module;

#include "entt/entt.hpp"

module systems;

import core;
import components;

void cooldown_run()
{
    auto ent = Player::ent;
    if (World::registry.valid(ent))
    {
        if (std::get<2>(Player::CoolDown::foothold))
        {
            auto delta_time = Window::delta_time;
            std::get<0>(Player::CoolDown::foothold) += delta_time;
            if (std::get<0>(Player::CoolDown::foothold) >= std::get<1>(Player::CoolDown::foothold))
            {
                std::get<0>(Player::CoolDown::foothold) = 0;
                std::get<2>(Player::CoolDown::foothold) = false;
            }
        }
    }
}
