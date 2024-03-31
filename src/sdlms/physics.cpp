#include "sdlms/physics.hpp"
#include <ranges>
#include <utility>

Physics::Physics()
{
    _map = Map::current();
    _character = Character::current();
}

void Physics::update(int elapsedTime)
{
    // auto fhs = _map->_foothold;
    // auto character_x = _character->_pos.a;
    // for (auto it : fhs)
    // {
    //     auto fh_l = it._a.x();
    //     auto fh_r = it._b.x();
    //     if (fh_l > fh_r)
    //     {
    //         std::exchange(fh_l, fh_r);
    //     }
    //     if (character_x >= fh_l && character_x <= fh_r)
    //     {
    //         /* code */
    //     }
        
    // }
    if (_character->_vforce!=0.0)
    {
        //垂直方向有力
        _character->_vacc=_character->_vforce;
        
    }

    return;
}
