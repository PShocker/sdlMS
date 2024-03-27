#include "sdlms/foothold.hpp"

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
