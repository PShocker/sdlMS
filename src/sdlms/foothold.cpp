#include "sdlms/foothold.hpp"
#include "util/map_util.hpp"

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
        _type = VERTICAL;
        if (next == 0 || prev == 0)
        {
            _disable = true;
        }
    }
    else if (a.y() == b.y())
    {
        // 地板
        _type = HORIZONTAL;
    }
    else
    {
        // 斜坡
        _k = ((float)b.y() - (float)a.y()) / ((float)b.x() - (float)a.x());
        _intercept = a.y() - _k * a.x();
        _type = BIAS;
    }

    _camera = Camera::current();
    _renderer = Graphics::current()->getRenderer();
}
void FootHold::draws(std::unordered_map<int, FootHold> &foothold)
{
    for (auto &[_, it] : foothold)
    {
        SDL_RenderDrawLine(it._renderer,
                           it._a.x() - it._camera->_viewport.x,
                           it._a.y() - it._camera->_viewport.y,
                           it._b.x() - it._camera->_viewport.x,
                           it._b.y() - it._camera->_viewport.y);
    }
}

std::optional<float> FootHold::get_x(float y)
{
    // 根据x坐标计算出斜线的y坐标,判断x在fh的范围内
    if (y == std::clamp(y, (float)_a.y(), (float)_b.y()))
    {
        switch (_type)
        {
        case HORIZONTAL:
            return std::nullopt;
        case VERTICAL:
            return _a.x();
        case BIAS:
            return (y - _intercept) / _k;
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
        case HORIZONTAL:
            return _a.y();
        case VERTICAL:
            return std::nullopt;
        case BIAS:
            return _k * x + _intercept;
        default:
            break;
        }
    }
    return std::nullopt;
}

std::unordered_map<int, FootHold> FootHold::load_foothold(int mapId)
{
    auto node = util::MapUtil::current()->load_map_node(mapId);

    node = node->get_child(u"foothold");
    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            auto page = std::stoi(std::string{it.first.begin(), it.first.end()});
            for (auto _it : it.second[0]->get_children())
            {
                auto zmass = std::stoi(std::string{_it.first.begin(), _it.first.end()});
                for (auto __it : _it.second[0]->get_children())
                {
                    auto id = std::stoi(std::string{__it.first.begin(), __it.first.end()});
                    auto foothold = __it.second[0];

                    auto next = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"next"))->get();
                    auto prev = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"prev"))->get();
                    auto x1 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"x1"))->get();
                    auto x2 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"x2"))->get();
                    auto y1 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"y1"))->get();
                    auto y2 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"y2"))->get();

                    FootHold f(Point(x1, y1), Point(x2, y2), page, zmass, id, prev, next);
                    if (f._disable != true)
                    {
                        _foothold.emplace(id, f);
                    }
                }
            }
        }
    }
    return _foothold;
}