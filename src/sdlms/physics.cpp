#include "sdlms/physics.hpp"
#include <ranges>
#include <utility>
#include <algorithm>

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
    float _elapsedTime = elapsedTime / 1000.0;

    // 垂直方向有力,默认质量为1
    _character->_hacc = _character->_hforce;
    _character->_vacc = _character->_vforce;

    auto _hspeed = _character->_hspeed + _elapsedTime * _character->_hacc;
    auto _vspeed = _character->_vspeed + _elapsedTime * _character->_vacc;

    Point<float> new_pos = _character->_pos + Point<float>{(_character->_hspeed + _hspeed) / 2 * _elapsedTime,
                                                           (_character->_vspeed + _vspeed) / 2 * _elapsedTime};

    _character->_hspeed += _elapsedTime * _character->_hacc;
    _character->_vspeed += _elapsedTime * _character->_vacc;

    _character->_pos = new_pos;
    if (_character->_vspeed >= 60)
    {
        _character->_vspeed = 60;
    }

    return;
}

// 判断线段是否相交
template <typename T>
inline std::optional<Point<T>> Physics::segmentsIntersection(std::pair<const Point<T> &, const Point<T> &> a, std::pair<const Point<T> &, const Point<T> &> b)
{
    // auto crossProduct = [](Point<T> a, Point<T> b) = >
    // {

    // }
    return std::optional<Point<T>>();
}
