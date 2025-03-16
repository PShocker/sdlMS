#include "Buff.h"
#include "entt/entt.hpp"
#include "Core/Core.h"

void buff_run()
{
    for (auto ent : World::registry->view<Buff>())
    {
        auto buff = World::registry->try_get<Buff>(ent);
        for (auto it = buff->buffs.begin(); it != buff->buffs.end();)
        {
            if (it->second.destory < Window::dt_now)
            {
                if (it->second.finish.has_value())
                {
                    it->second.finish.value()(buff->owner);
                }
                it = buff->buffs.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}
