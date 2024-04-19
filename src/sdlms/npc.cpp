#include "sdlms/npc.hpp"

Npc::Npc(EventSprite eventsprite, int fh) : EventSprite(eventsprite),
                                            _fh(fh),
                                            _gen(std::random_device()()),
                                            _dist(0, 0)

{
    for (const auto &[event, _] : eventsprite._eventsprite)
    {
        _v_event.push_back(event);
    }
    _dist.param(std::uniform_int_distribution<uint8_t>::param_type(0, _v_event.size() - 1));
}

void Npc::draw()
{
    _eventsprite.at(_event).draw();
}

void Npc::update(int elapsedTime)
{
    auto end = _eventsprite.at(_event).update(elapsedTime);
    if (end)
    {
        // 状态切换
        _event=_v_event[_dist(_gen)];
    }
}

void Npc::updates(std::vector<Npc> &npcs, int elapsedTime)
{
    for (auto &it : npcs)
    {
        // 更新帧状态
        it.update(elapsedTime);
    }
}

void Npc::draws(std::vector<Npc> &npcs)
{
    for (auto &it : npcs)
    {
        it.draw();
    }
}
