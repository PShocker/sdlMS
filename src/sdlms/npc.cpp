#include "sdlms/npc.hpp"

Npc::Npc(EventSprite eventsprite, int fh) : EventSprite(eventsprite),
                                            _fh(fh)
{
}

void Npc::draw()
{
    _eventsprite.at(_event).draw();
}

void Npc::update(int elapsedTime)
{
    auto &_dynamicsprite = _eventsprite.at(_event)._dynamicsprite;
    auto &_animatedsprite = std::get<AnimatedSprite>(_dynamicsprite);

    _animatedsprite._frameTime += elapsedTime;
    if (_animatedsprite._frameTime >= _animatedsprite._delay[_animatedsprite._frameIndex])
    {
        if (_animatedsprite._frameIndex == _animatedsprite._frameSize - 1)
        {
            _animatedsprite._frameIndex = 0;
            if (_eventsprite.size() > 1)
            {
                // 状态切换
                auto it = _eventsprite.find(_event);
                auto next_it = std::next(it);
                if (next_it != _eventsprite.end())
                {
                    _event = next_it->first;
                }
                else
                {
                    _event = _eventsprite.begin()->first;
                }
            }
        }
        else
        {
            _animatedsprite._frameIndex += 1;
        }
        // 切换下一帧
        _animatedsprite._frameTime = 0;
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
