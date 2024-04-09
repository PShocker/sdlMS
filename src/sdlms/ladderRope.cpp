#include "sdlms/ladderRope.hpp"

LadderRope::LadderRope()
{
}
LadderRope::LadderRope(int id, int l, int uf, int x, int y1, int y2, int page) : _id(id), _l(l), _uf(uf),
                                                                                 _x(x), _y1(y1), _y2(y2), _page(page)
{
    _camera = Camera::current();
    _renderer = Graphics::current()->getRenderer();
}
void LadderRope::draws(std::unordered_map<int, LadderRope> r)
{
    for (auto [_, it] : r)
    {
        SDL_RenderDrawLine(it._renderer,
                           it._x - it._camera->_viewport.x,
                           it._y1 - it._camera->_viewport.y,
                           it._x - it._camera->_viewport.x,
                           it._y2 - it._camera->_viewport.y);
    }
}