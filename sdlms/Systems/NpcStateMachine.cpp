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
    auto foo = mv->foo;
    if (!move_move(mv, tr, 0, delta_time) && mv->foo == nullptr)
    {
        // 不掉落
        mv->foo = foo;
        tr->position.x = std::clamp(tr->position.x, (float)foo->l, (float)foo->r);
        tr->position.y = foo->get_y(tr->position.x).value();
    }
    return;
}
