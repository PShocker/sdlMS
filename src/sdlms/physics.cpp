#include "sdlms/physics.hpp"
#include <ranges>
#include <utility>
#include <algorithm>
#include "physics.hpp"

Physics::Physics()
{
    _map = Map::current();
    _character = Character::current();
    _input = Input::current();
}

void Physics::move_ground(float elapsedTime)
{
    auto fhs = FootHold::_foothold;

    auto friction = 800;
    if (_input->isKeyHeld(SDL_SCANCODE_UP) || _input->isKeyHeld(SDL_SCANCODE_DOWN))
    {
        if (move_ladderope())
        {
            return;
        }
    }
    if (_input->isKeyHeld(SDL_SCANCODE_RIGHT))
    {
        _character->_hforce = 1400;
        _character->switch_type(Character::Type::WALK1);
    }
    else if (_input->isKeyHeld(SDL_SCANCODE_LEFT))
    {
        _character->_hforce = -1400;
        _character->switch_type(Character::Type::WALK1);
    }
    else
    {
        _character->switch_type(Character::Type::STAND2);
    }
    if (_input->isKeyHeld(SDL_SCANCODE_DOWN))
    {
        _character->_hspeed = 0;
        _character->switch_type(Character::Type::PRONE);
        return;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_LALT))
    {
        // 跳跃
        _physic_status = AIR;
        _character->_vspeed = -555;
        _character->switch_type(Character::Type::JUMP);
        return;
    }

    // 地面移动判断
    //  摩擦力
    if (std::abs(_character->_hforce) < friction) // 合力小于摩擦力
    {
        if (_character->_hspeed > 0)
        {
            // 摩擦力左
            _character->_hforce -= friction;
            _character->_hforce = std::max(-_character->_hspeed / elapsedTime, _character->_hforce);
        }
        else
        {
            // 摩擦力右
            _character->_hforce += friction;
            _character->_hforce = std::min(-_character->_hspeed / elapsedTime, _character->_hforce);
        }
    }
    else // 合力大于摩擦力
    {
        if (_character->_hspeed > 0)
        {
            _character->_hforce -= friction;
        }
        else
        {
            _character->_hforce += friction;
        }
    }

    _character->_hacc = _character->_hforce;

    auto _hspeed = _character->_hspeed + elapsedTime * _character->_hacc;
    _hspeed = std::clamp(_hspeed, -125.0f, 125.0f);

    if (std::abs(_hspeed) < 0.1)
    {
        // 如果速度太小,则直接设置0,避免浮点数精度问题
        _hspeed = 0;
    }

    auto x = _character->_pos.x() + (_character->_hspeed + _hspeed) / 2 * elapsedTime;
    auto y = _character->_pos.y();

    // 切换fh
    // 往左走
    while (x < std::min(_fh._a.x(), _fh._b.x()))
    {
        FootHold fh;
        if (fhs.contains(_fh._prev))
        {
            fh = fhs[_fh._prev];
        }
        else
        {
            // 从fh掉落
            _physic_status = AIR;
            break;
        }
        if (fh._type == FootHold::VERTICAL)
        {
            if (y > std::min(fh._a.y(), fh._b.y()))
            {
                // 撞墙,人物x不变
                x = _character->_pos.x();
                _hspeed = 0;
                break;
            }
            else
            {
                // 从fh掉落
                _physic_status = AIR;
                break;
            }
        }
        _fh = fh;
    }
    // 往右走
    while (x > std::max(_fh._a.x(), _fh._b.x()))
    {
        FootHold fh;
        if (fhs.contains(_fh._next))
        {
            fh = fhs[_fh._next];
        }
        else
        {
            // 从fh掉落
            _physic_status = AIR;
            break;
        }
        if (fh._type == FootHold::VERTICAL)
        {
            // 撞墙
            if (y > std::min(fh._a.y(), fh._b.y()))
            {
                x = _character->_pos.x();
                _hspeed = 0;
                break;
            }
            else
            {
                // 从fh掉落
                _physic_status = AIR;
                break;
            }
        }
        _fh = fh;
    }
    if (_fh.get_y(x).has_value())
    {
        _character->_pos.set_y(_fh.get_y(x).value());
    }
    else
    {
        _character->_pos.set_y(y);
    }
    _character->_pos.set_x(x);

    _character->_vspeed = 0;
    _character->_hspeed = _hspeed;

    return;
}

void Physics::move_ladderope(float elapsedTime)
{
    _character->_animate = false;

    if (_input->isKeyHeld(SDL_SCANCODE_LALT))
    {
        if (_input->isKeyHeld(SDL_SCANCODE_RIGHT))
        {
            _character->_hspeed = 100;
            _character->_vspeed = -300;
            _physic_status = AIR;
            _character->_animate = true;
            return;
        }
        else if (_input->isKeyHeld(SDL_SCANCODE_LEFT))
        {
            _character->_hspeed = -100;
            _character->_vspeed = -300;
            _physic_status = AIR;
            _character->_animate = true;
            return;
        }
    }
    auto _vspeed = 0;
    if (_input->isKeyHeld(SDL_SCANCODE_UP))
    {
        _character->_animate = true;
        _vspeed = -100;
    }
    else if (_input->isKeyHeld(SDL_SCANCODE_DOWN))
    {
        _character->_animate = true;
        _vspeed = 100;
    }

    auto y = _character->_pos.y() + _vspeed * elapsedTime;
    if (y <= _lp._y1)
    {
        y = _lp._y1 - 5;
        _physic_status = AIR;
        _character->_animate = true;
    }
    else if (y > _lp._y2)
    {
        _physic_status = AIR;
        _character->_animate = true;
    }
    _character->_pos.set_y(y);
    return;
}

bool Physics::move_ladderope()
{
    auto lps = LadderRope::_ladderRope;
    std::unordered_map<int, LadderRope>::iterator lp;

    auto y = _character->_pos.y();

    if (_input->isKeyHeld(SDL_SCANCODE_UP))
    {
        lp = std::ranges::find_if(lps, [this](const auto &pair)
                                  { return (std::abs(this->_character->_pos.x() - pair.second._x) < 10) &&
                                           (this->_character->_pos.y() == std::clamp(this->_character->_pos.y(), (float)pair.second._y1 + 5, (float)pair.second._y2)); });
    }
    else if (_input->isKeyHeld(SDL_SCANCODE_DOWN))
    {
        lp = std::ranges::find_if(lps, [this](const auto &pair)
                                  { return (std::abs(this->_character->_pos.x() - pair.second._x) < 10) &&
                                           (std::abs((float)pair.second._y1 - this->_character->_pos.y()) < 5); });
        y += 5;
    }
    if (lp != lps.end())
    {
        _character->_pos.set_x(lp->second._x);
        _character->_pos.set_y(y);
        // 判断爬的是梯子还是绳子
        if (lp->second._l == 1)
        {
            // 梯子
            _character->switch_type(Character::Type::LADDER);
        }
        else
        {
            // 绳子
            _character->switch_type(Character::Type::ROPE);
        }
        _character->_animate = false;
        _character->_vspeed = 0;
        _character->_hspeed = 0;
        _lp = lp->second;
        _character->_layer = _lp._page;
        _physic_status = CLIMB;
        return true;
    }
    return false;
}

void Physics::move_air(float elapsedTime)
{
    if (_input->isKeyHeld(SDL_SCANCODE_UP))
    {
        if (move_ladderope())
        {
            return;
        }
    }

    auto fhs = FootHold::_foothold;

    _character->switch_type(Character::Type::JUMP);

    _character->_vacc = _character->_vforce = 2000;

    auto _hspeed = _character->_hspeed + elapsedTime * _character->_hacc;

    auto _vspeed = _character->_vspeed + elapsedTime * _character->_vacc;
    _vspeed = std::min(_vspeed, 670.0f);

    Point<float> new_pos = _character->_pos + Point<float>{(_character->_hspeed + _hspeed) / 2 * elapsedTime,
                                                           (_character->_vspeed + _vspeed) / 2 * elapsedTime};

    // 判断上升还是下降
    auto raise = new_pos.y() < _character->_pos.y() ? true : false;

    for (const auto &[id, it] : fhs | std::views::filter([](const auto &pair)
                                                         {
                                                             return (pair.second._type == FootHold::VERTICAL); // 先处理与墙的碰撞
                                                         }))
    {
        if (it._disable == true || _character->_layer != it._page)
        {
            continue;
        }

        if (std::min(_character->_pos.x(), new_pos.x()) > std::max(it._a.x(), it._b.x()))
        {
            continue;
        }
        if (std::max(_character->_pos.x(), new_pos.x()) < std::min(it._a.x(), it._b.x()))
        {
            continue;
        }

        Point<float> p1 = {(float)it._a.x(), (float)it._a.y()};
        Point<float> p2 = {(float)it._b.x(), (float)it._b.y()};

        std::pair<const Point<float> &, const Point<float> &> l1(_character->_pos, new_pos);
        std::pair<const Point<float> &, const Point<float> &> l2(p1, p2);

        auto r = segmentsIntersection(l1, l2);
        if (r.has_value())
        {
            // 修改坐标为交点
            auto intersect_pos = r.value();
            // 从空中撞到墙
            _character->_hspeed = 0.0f;
            intersect_pos.set_x(_character->_pos.x());
            intersect_pos.set_y(new_pos.y());
            new_pos = intersect_pos;
            break;
        }
    }
    if (raise == false)
    {
        for (const auto &[id, it] : fhs | std::views::filter([](const auto &pair)
                                                             {
                                                                 return pair.second._type != FootHold::VERTICAL; // 后处理与墙面,斜坡碰撞
                                                             }))
        {
            if (std::min(_character->_pos.x(), new_pos.x()) > std::max(it._a.x(), it._b.x()))
            {
                continue;
            }
            if (std::max(_character->_pos.x(), new_pos.x()) < std::min(it._a.x(), it._b.x()))
            {
                continue;
            }

            Point<float> p1 = {(float)it._a.x(), (float)it._a.y()};
            Point<float> p2 = {(float)it._b.x(), (float)it._b.y()};

            std::pair<const Point<float> &, const Point<float> &> l1(_character->_pos, new_pos);
            std::pair<const Point<float> &, const Point<float> &> l2(p1, p2);

            auto r = segmentsIntersection(l1, l2);
            if (r.has_value())
            {
                auto intersect_pos = r.value();
                _fh = it;
                _physic_status = GROUND;
                _character->_vspeed = 0.0f;
                // _character->_hspeed /= 2;
                _character->_layer = _fh._page;
                new_pos = intersect_pos;
                break;
            }
        }
    }
    _character->_pos = new_pos;
    _character->_vspeed = _vspeed;
}

void Physics::direct()
{
    if (_input->isKeyHeld(SDL_SCANCODE_RIGHT))
    {
        _character->_direct = false;
    }
    else if (_input->isKeyHeld(SDL_SCANCODE_LEFT))
    {
        _character->_direct = true;
    }
}

void Physics::update(int elapsedTime)
{
    float _elapsedTime = elapsedTime / 1000.0;
    _character->_hforce = _character->_hacc = 0;
    _character->_vforce = _character->_vacc = 0;
    direct();
    switch (_physic_status)
    {
    case GROUND:
        move_ground(_elapsedTime);
        break;
    case CLIMB:
        move_ladderope(_elapsedTime);
        break;
    case AIR:
        move_air(_elapsedTime);
        break;
    default:
        break;
    }
    return;
}

// 判断线段是否相交
template <typename T>
inline std::optional<Point<T>> Physics::segmentsIntersection(std::pair<const Point<T> &, const Point<T> &> a, std::pair<const Point<T> &, const Point<T> &> b)
{
    std::optional<Point<T>> intersection;

    auto [p1, p2] = a;
    auto [p3, p4] = b;
    // 快速排斥实验
    if ((p1.x() > p2.x() ? p1.x() : p2.x()) < (p3.x() < p4.x() ? p3.x() : p4.x()) ||
        (p1.y() > p2.y() ? p1.y() : p2.y()) < (p3.y() < p4.y() ? p3.y() : p4.y()) ||
        (p3.x() > p4.x() ? p3.x() : p4.x()) < (p1.x() < p2.x() ? p1.x() : p2.x()) ||
        (p3.y() > p4.y() ? p3.y() : p4.y()) < (p1.y() < p2.y() ? p1.y() : p2.y()))
    {
        return intersection;
    }
    // 跨立实验
    if ((((p1.x() - p3.x()) * (p4.y() - p3.y()) - (p1.y() - p3.y()) * (p4.x() - p3.x())) *
         ((p2.x() - p3.x()) * (p4.y() - p3.y()) - (p2.y() - p3.y()) * (p4.x() - p3.x()))) > 0 ||
        (((p3.x() - p1.x()) * (p2.y() - p1.y()) - (p3.y() - p1.y()) * (p2.x() - p1.x())) *
         ((p4.x() - p1.x()) * (p2.y() - p1.y()) - (p4.y() - p1.y()) * (p2.x() - p1.x()))) > 0)
    {
        return intersection;
    }

    auto x = ((p1.y() - p3.y()) * (p2.x() - p1.x()) * (p4.x() - p3.x()) + p3.x() * (p4.y() - p3.y()) * (p2.x() - p1.x()) - p1.x() * (p2.y() - p1.y()) * (p4.x() - p3.x())) / ((p4.x() - p3.x()) * (p1.y() - p2.y()) - (p2.x() - p1.x()) * (p3.y() - p4.y()));
    auto y = (p2.y() * (p1.x() - p2.x()) * (p4.y() - p3.y()) + (p4.x() - p2.x()) * (p4.y() - p3.y()) * (p1.y() - p2.y()) - p4.y() * (p3.x() - p4.x()) * (p2.y() - p1.y())) / ((p1.x() - p2.x()) * (p4.y() - p3.y()) - (p2.y() - p1.y()) * (p3.x() - p4.x()));

    intersection.emplace(Point<float>{x, y});

    return intersection;
}
