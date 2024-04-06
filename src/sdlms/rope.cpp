#include "sdlms/rope.hpp"

Rope::Rope()
{
}
Rope::Rope(int id, int l, int uf, int x, int y1, int y2, int page) : _id(id), _l(l), _uf(uf),
                                                                     _x(x), _y1(y1), _y2(y2), _page(page)
{
    _camera = Camera::current();
    _renderer = Graphics::current()->getRenderer();
}
void Rope::draw()
{
    SDL_RenderDrawLine(_renderer,
                       _x - _camera->_viewport.x,
                       _y1 - _camera->_viewport.y,
                       _x - _camera->_viewport.x,
                       _y2 - _camera->_viewport.y);
}