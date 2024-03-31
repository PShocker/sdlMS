#include "sdlms/foothold.hpp"
#include "foothold.hpp"

FootHold::FootHold(Point<int32_t> a, Point<int32_t> b, int page, int zmass, int id, int prev, int next) : _a(a),
                                                                                                          _b(b),
                                                                                                          _page(page),
                                                                                                          _zmass(zmass),
                                                                                                          _id(id),
                                                                                                          _prev(prev), _next(next)
{
    _len = b.distance(a);
    _uvx = (b.x() - a.x()) / _len;
    _uvy = (b.y() - a.y()) / _len;

    if (a.x() == b.x())
    {
        // 墙面
        _type = WALL;
    }
    else if (a.y() == b.y())
    {
        // 地板
        _type = FLOOR;
    }
    else
    {
        // 斜坡
        _k = (b.y() - a.y()) / (b.x() - a.x() * 1.0);
        _c = (a.y() * b.x() - b.y() * a.x()) / (b.x() - a.x() * 1.0);
    }

    _camera = Camera::current();
    _renderer = Graphics::current()->getRenderer();
}
void FootHold::draw()
{
    SDL_RenderDrawLine(_renderer,
                       _a.x() - _camera->_viewport.x,
                       _a.y() - _camera->_viewport.y,
                       _b.x() - _camera->_viewport.x,
                       _b.y() - _camera->_viewport.y);
}

float FootHold::get_x(float y)
{
    return 0.0f;
}

float FootHold::get_y(float x)
{
    // 根据x坐标计算出斜线的y坐标
    switch (_type)
    {
    case FLOOR:
        return _a.y();
    case WALL:
        return 0.0f;
    case SLOPE:
        return _k * x + _c;
    default:
        break;
    }
    return 0.0f;
}
