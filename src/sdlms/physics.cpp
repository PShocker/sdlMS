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
    elapsedTime = elapsedTime / 1000.0;
    if (_character->_vforce != 0.0)
    {
        // 垂直方向有力,默认质量为1
        _character->_vacc = _character->_vforce;
        auto _y = _character->_pos.b + _character->_vspeed * elapsedTime + 0.5 * _character->_vacc * elapsedTime * elapsedTime;
        _character->_vspeed += elapsedTime * _character->_vacc;
        if (_character->_vspeed >= 10)
        {
            _character->_vspeed = 10;
        }

        _character->_pos.b = _y;
    }

    return;
}
