module;

#include "entt/entt.hpp"

module systems;

import core;
import components;

void cooldown_run()
{
    if (auto ent = Player::ent; World::registry->valid(ent))
    {
        if (Player::CoolDowns::foothold.start)
        {
            auto delta_time = Window::delta_time;
            Player::CoolDowns::foothold.current += delta_time;
            if (Player::CoolDowns::foothold.current >= Player::CoolDowns::foothold.delay)
            {
                Player::CoolDowns::foothold.current = 0;
                Player::CoolDowns::foothold.start = false;
            }
        }
    }
}
