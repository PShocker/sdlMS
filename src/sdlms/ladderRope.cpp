#include "sdlms/ladderRope.hpp"
#include "util/map_util.hpp"
#include "sdlms/foothold.hpp"
#include "util/wz_util.hpp"

LadderRope::LadderRope()
{
}
LadderRope::LadderRope(int id, int l, int uf, int x, int y1, int y2, int page) : _id(id), _l(l), _uf(uf),
                                                                                 _x(x), _y1(y1), _y2(y2), _page(page)
{
    _camera = Camera::current();
    _renderer = Graphics::current()->getRenderer();
}
void LadderRope::draws(std::unordered_map<int, LadderRope> &r)
{
    for (auto &[_, it] : r)
    {
        SDL_RenderDrawLine(it._renderer,
                           it._x - it._camera->_viewport.x,
                           it._y1 - it._camera->_viewport.y,
                           it._x - it._camera->_viewport.x,
                           it._y2 - it._camera->_viewport.y);
    }
}
void LadderRope::load_ladderRope(int mapId)
{
    auto node=util::MapUtil::current()->load_map_node(mapId);

    node = node->get_child(u"ladderRope");
    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            auto id = std::stoi(std::string{it.first.begin(), it.first.end()});
            auto rope = it.second[0];

            auto l = dynamic_cast<wz::Property<int> *>(rope->get_child(u"l"))->get();
            auto uf = dynamic_cast<wz::Property<int> *>(rope->get_child(u"uf"))->get();
            auto x = dynamic_cast<wz::Property<int> *>(rope->get_child(u"x"))->get();
            auto y1 = dynamic_cast<wz::Property<int> *>(rope->get_child(u"y1"))->get();
            auto y2 = dynamic_cast<wz::Property<int> *>(rope->get_child(u"y2"))->get();
            auto page = dynamic_cast<wz::Property<int> *>(rope->get_child(u"page"))->get();

            LadderRope r(id, l, uf, x, y1, y2, page);

            _ladderRope.emplace(id, r);
        }
    }
}
