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
            if (it->second.destory < Window::dt_now || (it->second.duration == -1 && it->second.destory != 0))
            {
                if (it->second.finish.has_value())
                {
                    it->second.finish.value()(buff->owner);
                }
                it = buff->buffs.erase(it);
            }
            else
            {
                if (it->second.frame.has_value())
                {
                    it->second.frame.value()(buff->owner);
                }
                ++it;
            }
        }
    }
}
