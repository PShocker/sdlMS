#include "sdlms/foothold.hpp"

FootHold::FootHold()
{
}

FootHold::FootHold(Point<int32_t> a, Point<int32_t> b, int page, int zmass, int id, int prev, int next) : _a(a),
                                                                                                          _b(b),
                                                                                                          _page(page),
                                                                                                          _zmass(zmass),
                                                                                                          _id(id),
                                                                                                          _prev(prev), _next(next)
{
    _len = b.distance(a);
    _uvx = (float)(b.x() - a.x()) / _len;
    _uvy = (float)(b.y() - a.y()) / _len;

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
        _k = (float)(b.y() - a.y()) / (b.x() - a.x() * 1.0);
        _c = (float)(a.y() * b.x() - b.y() * a.x()) / (b.x() - a.x() * 1.0);
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

std::optional<float> FootHold::get_x(float y)
{
    // 根据x坐标计算出斜线的y坐标,判断x在fh的范围内
    if (y == std::clamp(y, (float)_a.y(), (float)_b.y()))
    {
        switch (_type)
        {
        case FLOOR:
            return std::nullopt;
        case WALL:
            return _a.x();
        case SLOPE:
            return (y - _c) / _k;
        default:
            break;
        }
    }
    return std::nullopt;
}

std::optional<float> FootHold::get_y(float x)
{
    // 根据x坐标计算出斜线的y坐标,判断x在fh的范围内
    if (x == std::clamp(x, (float)_a.x(), (float)_b.x()))
    {
        switch (_type)
        {
        case FLOOR:
            return _a.y();
        case WALL:
            return std::nullopt;
        case SLOPE:
            return _k * x + _c;
        default:
            break;
        }
    }
    return std::nullopt;
}
