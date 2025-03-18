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
            if (it->second.duration == 0)
            {
                // 永久型buff,除非手动取消
                if (it->second.destory != 0)
                {
                    if (it->second.finish.has_value())
                    {
                        it->second.finish.value()(buff->owner);
                    }
                    it = buff->buffs.erase(it);
                    continue;
                }
            }
            else if (it->second.destory < Window::dt_now)
            {
                if (it->second.finish.has_value())
                {
                    it->second.finish.value()(buff->owner);
                }
                it = buff->buffs.erase(it);
                continue;
            }
            if (it->second.frame.has_value())
            {
                it->second.frame.value()(buff->owner);
            }
            ++it;
        }
    }
}
