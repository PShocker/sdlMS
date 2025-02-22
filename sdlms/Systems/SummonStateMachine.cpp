#include "SummonStateMachine.h"
#include "PlayerSummon/PlayerSummon.h"

void summon_statemachine_run()
{
    for (auto ent : World::registry->view<Summon>())
    {
        auto sum = World::registry->try_get<Summon>(ent);
        if (sum->destory > Window::dt_now)
        {
            if (PlayerSummon::Summons.contains(sum->id))
            {
                PlayerSummon::Summons.at(sum->id)(ent);
            }
        }
        else
        {
            World::destory.push_back(ent);
            World::zindex = true;
        }
    }
}