#include <cmath>
#include <vector>
#include <algorithm>
#include <ranges>
#include "sdlms/backgrd.hpp"
#include "util/map_util.hpp"
#include "util/wz_util.hpp"

BackGrd::BackGrd(std::variant<Sprite, AnimatedSprite> dynamicsprite,
                 int id, int type,
                 int front,
                 int rx, int ry,
                 int cx, int cy,
                 int ani, std::u16string url) : DynamicSprite(dynamicsprite),
                                                _id(id), _type(type),
                                                _front(front),
                                                _rx(rx), _ry(ry),
                                                _cx(cx), _cy(cy),
                                                _ani(ani),
                                                _position_offset_x(0.0), _position_offset_y(0.0),
                                                _url(url)
{
    _camera = Camera::current();
}

void BackGrd::update(int elapsedTime)
{
    _backgrds.clear();

    auto htile = 0; // 水平平铺
    auto vtile = 0; // 垂直平铺

    auto hspeed = 0; // 水平移动
    auto vspeed = 0; // 垂直移动
    switch (_type)
    {
    case Type::HTILED:
    case Type::HMOVEA:
        htile = 1;
        break;
    case Type::VTILED:
    case Type::VMOVEA:
        vtile = 1;
        break;
    case Type::TILED:
    case Type::HMOVEB:
    case Type::VMOVEB:
        htile = 1;
        vtile = 1;
        break;
    }
    switch (_type)
    {
    case Type::HMOVEA:
    case Type::HMOVEB:
        hspeed = 1;
        break;
    case Type::VMOVEA:
    case Type::VMOVEB:
        vspeed = 1;
        break;
    }

    auto viewprot = _camera->_viewport;

    const Sprite *s;

    if (std::holds_alternative<AnimatedSprite>(_dynamicsprite))
    {
        std::get<AnimatedSprite>(_dynamicsprite).update(elapsedTime);
        s = &std::get<AnimatedSprite>(_dynamicsprite)._sprite[std::get<AnimatedSprite>(_dynamicsprite)._frameIndex];
    }
    else if (std::holds_alternative<Sprite>(_dynamicsprite))
    {
        s = &std::get<Sprite>(_dynamicsprite);
    }

    auto tile_cnt_x = 1;
    auto tile_cnt_y = 1;
    SDL_FPoint point = {s->_rect.x, s->_rect.y};
    if (hspeed > 0)
    {
        _position_offset_x = fmodf(_position_offset_x + _rx * 5 * elapsedTime / 1000.0, _cx);
    }
    else
    {
        _position_offset_x = (viewprot.x + viewprot.w / 2) * (_rx + 100) / 100.0;
    }

    if (vspeed > 0)
    {
        _position_offset_y = fmodf(_position_offset_y + _ry * 5 * elapsedTime / 1000.0, _cy);
    }
    else
    {
        _position_offset_y = (viewprot.y + viewprot.h / 2) * (_ry + 100) / 100.0;
    }
    point.x += _position_offset_x;
    point.y += _position_offset_y;

    if (htile > 0 && _cx > 0)
    {
        auto tile_start_right = int(point.x + s->_rect.w - viewprot.x) % _cx;
        if (tile_start_right <= 0)
        {
            tile_start_right = tile_start_right + _cx;
        }
        tile_start_right = tile_start_right + viewprot.x;

        auto tile_start_left = tile_start_right - s->_rect.w;
        if (tile_start_left >= viewprot.x + viewprot.w)
        {
            tile_cnt_x = 0;
        }
        else
        {
            tile_cnt_x = ceil((viewprot.x + viewprot.w - tile_start_left) / float(_cx));
            point.x = tile_start_left;
        }
    }

    if (vtile > 0 && _cy > 0)
    {
        auto tile_start_bottom = int(point.y + s->_rect.h - viewprot.y) % _cy;
        if (tile_start_bottom <= 0)
        {
            tile_start_bottom = tile_start_bottom + _cy;
        }
        tile_start_bottom = tile_start_bottom + viewprot.y;

        auto tile_start_top = tile_start_bottom - s->_rect.h;
        if (tile_start_top >= viewprot.y + viewprot.h)
        {
            tile_cnt_y = 0;
        }
        else
        {
            tile_cnt_y = ceil((viewprot.y + viewprot.h - tile_start_top) / float(_cy));
            point.y = tile_start_top;
        }
    }

    for (int i = 0; i < tile_cnt_y; i++)
    {
        for (int j = 0; j < tile_cnt_x; j++)
        {
            SDL_FRect rect{point.x + j * _cx, point.y + i * _cy, s->_rect.w, s->_rect.h};
            _backgrds.push_back(DynamicSprite(Sprite(s->_texture, rect, s->_flip)));
        }
    }
}

void BackGrd::updates(int elapsedTime)
{
    for (auto &it : _backgrd.first)
    {
        it.update(elapsedTime);
    }
    for (auto &it : _backgrd.second)
    {
        it.update(elapsedTime);
    }
}
void BackGrd::drawbackgrounds()
{
    // 绘制背景
    for (auto &it : _backgrd.first)
    {
        for (auto &_it : it._backgrds)
        {
            _it.draw();
        }
    }
}

void BackGrd::drawforegrounds()
{
    // 绘制背景
    for (auto &it : _backgrd.second)
    {
        for (auto &_it : it._backgrds)
        {
            _it.draw();
        }
    }
}

void BackGrd::load_backgrd(int mapId)
{
    auto node = util::MapUtil::current()->load_map_node(mapId);
    std::vector<BackGrd> v_backgrd;
    node = node->get_child(u"back");

    auto _map_node = util::WzUtil::current()->Map->get_root();

    if (node != nullptr)
    {
        for (auto it : node->get_children())
        {
            auto bS = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"bS"))->get();
            if (bS == u"")
            {
                break;
            }
            auto ani = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"ani"))->get();

            auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
            auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"y"))->get();

            auto cx = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"cx"))->get();
            auto cy = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"cy"))->get();

            auto rx = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"rx"))->get();
            auto ry = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"ry"))->get();

            auto type = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"type"))->get();

            auto no = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"no"))->get();

            auto front = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"front"))->get();

            auto id = std::stoi(std::string{it.first.begin(), it.first.end()});
            auto flip = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"f"))->get();

            switch (ani)
            {
            case 0:
            {
                auto no_str = std::to_string(no);
                auto url = u"Back/" + bS + u".img/" + u"back" + u"/" + std::u16string{no_str.begin(), no_str.end()};
                auto back = _map_node->find_from_path(url);
                if (back != nullptr)
                {
                    Sprite sprite = Sprite::load_sprite(back, x, y, flip);

                    cx = cx == 0 ? sprite._rect.w : cx;
                    cy = cy == 0 ? sprite._rect.h : cy;

                    BackGrd backgrd(sprite, id, type, front, rx, ry, cx, cy, ani, url);

                    v_backgrd.push_back(backgrd);
                    break;
                }
            }
            case 1:
            {
                auto no_str = std::to_string(no);
                auto url = u"Back/" + bS + u".img/" + u"ani" + u"/" + std::u16string{no_str.begin(), no_str.end()};
                auto animatedsprite = AnimatedSprite::load_animated_sprite(_map_node->find_from_path(url), x, y, flip);
                BackGrd backgrd(animatedsprite, id, type, front, rx, ry, cx, cy, ani, url);
                v_backgrd.push_back(backgrd);
                break;
            }
            default:
                break;
            }
        }
    }
    std::ranges::sort(v_backgrd, [](const BackGrd a, const BackGrd b)
                      { return a._id < b._id; });

    std::vector<BackGrd> v_back_backgrd;
    for (auto it : v_backgrd | std::views::filter([](BackGrd b)
                                                  { return b._front == 0; }))
    {
        v_back_backgrd.push_back(it);
    }

    std::vector<BackGrd> v_fore_backgrd;
    for (auto it : v_backgrd | std::views::filter([](BackGrd b)
                                                  { return b._front == 1; }))
    {
        v_fore_backgrd.push_back(it);
    }
    _backgrd= {v_back_backgrd, v_fore_backgrd};
}
