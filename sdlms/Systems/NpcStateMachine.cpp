#include "NpcStateMachine.h"
#include "Move.h"

void npc_statemachine_run()
{
    auto view = World::registry->view<Npc>();
    for (auto ent : view)
    {
        npc_statemachine(ent, (float)Window::delta_time / 1000);
    }
}

void npc_statemachine(entt::entity ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto npc = World::registry->try_get<Npc>(ent);
    if (npc->index == u"move")
    {
        npc_move(mv, tr, delta_time);
    }
}

void npc_move(Move *mv, Transform *tr, float delta_time)
{
    move_move(mv, tr, 0, delta_time, false);
    return;
}
