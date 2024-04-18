#include "sdlms/npc.hpp"

Npc::Npc(std::unordered_map<Event, DynamicSprite> eventsprite, int fh) : _eventsprite(eventsprite),
                                                                 _fh(fh)
{
}

void Npc::draws(std::vector<Npc> &npcs)
{
    // for (auto &it : npcs)
    // {
    //     it.draw();
    // }
}
