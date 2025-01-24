#include "CoolDown.h"
#include "entt/entt.hpp"
#include "Core/Core.h"

void cooldown_run()
{
    auto delta_time = Window::delta_time;
    auto &cooldowns = SkillWarp::cooldowns;
    for (auto it = cooldowns.begin(); it != cooldowns.end();)
    {
        it->second -= delta_time;
        if (it->second <= 0)
        {                             
            it = cooldowns.erase(it); // erase 返回下一个有效的迭代器
        }
        else
        {
            ++it; // 仅当没有删除时才递增迭代器
        }
    }
}
