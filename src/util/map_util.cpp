#include "util/map_util.hpp"
#include "util/wz_util.hpp"
#include "sdlms/graphics.hpp"
#include <SDL2/SDL.h>
#include <string>

namespace util
{
    MapUtil::MapUtil()
    {
        _sprite_util = SpriteUtil::current();
        _map_node = WzUtil::current()->Map->get_root();
    }

    std::array<std::vector<Tile>, 8> MapUtil::load_tile(int mapId)
    {
        std::array<std::vector<Tile>, 8> tile;
        for (size_t i = 0; i < 8; i++)
        {
            tile[i] = load_tile(load_node(mapId), i);
        }
        return tile;
    }

    std::vector<Tile> MapUtil::load_tile(wz::Node *node, int i)
    {
        std::vector<Tile> tile;
        node = node->get_child(std::to_string(i));
        auto tS = node->get_child(u"info")->get_child(u"tS");
        if (tS != nullptr)
        {
            for (auto it : node->get_child(u"tile")->get_children())
            {
                auto u = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"u"))->get();
                auto no = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"no"))->get();
                auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
                auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"y"))->get();

                auto no_str = std::to_string(no);

                auto url = u"Tile/" + dynamic_cast<wz::Property<wz::wzstring> *>(tS)->get() + u".img/" + u + u"/" + std::u16string{no_str.begin(), no_str.end()};

                auto tn = _map_node->find_from_path(url);

                Sprite sprite = _sprite_util->load_sprite(tn, x, y);

                auto z = dynamic_cast<wz::Property<int> *>(tn->get_child(u"z"))->get();

                Tile t(sprite, i, z);

                tile.push_back(t);
            }
        }
        std::ranges::sort(tile, [](const Tile a, const Tile b)
                          { return a._z < b._z; });
        return tile;
    }

    std::array<std::vector<Obj>, 8> MapUtil::load_obj(int mapId)
    {
        std::array<std::vector<Obj>, 8> obj;
        for (size_t i = 0; i < 8; i++)
        {
            obj[i] = load_obj(load_node(mapId), i);
        }
        return obj;
    }

    std::vector<Obj> MapUtil::load_obj(wz::Node *node, int i)
    {
        std::vector<Obj> obj;
        node = node->get_child(std::to_string(i))->get_child(u"obj");
        for (auto it : node->get_children())
        {
            std::vector<Sprite> v_sprite;
            std::vector<int> v_delay;
            std::vector<std::tuple<int, int>> v_a;
            auto oS = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"oS"))->get();
            auto l0 = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"l0"))->get();
            auto l1 = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"l1"))->get();
            auto l2 = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"l2"))->get();
            auto url = u"Obj/" + oS + u".img/" + l0 + u"/" + l1 + u"/" + l2;

            auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
            auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"y"))->get();
            auto z = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"z"))->get();

            auto flip = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"f"))->get();

            auto animatedsprite = _sprite_util->load_animated_sprite(_map_node->find_from_path(url), x, y, flip);

            Obj o(animatedsprite, i, z, url);
            obj.push_back(o);
        }
        std::ranges::sort(obj, [](const Obj a, const Obj b)
                          { return a._z < b._z; });
        return obj;
    }

    std::vector<BackGrd> MapUtil::load_backgrd(int mapId)
    {
        return load_backgrd(load_node(mapId));
    }

    std::vector<BackGrd> MapUtil::load_backgrd(wz::Node *node)
    {
        std::vector<BackGrd> v_backgrd;
        node = node->get_child(u"back");
        if (node != nullptr)
        {
            for (auto it : node->get_children())
            {
                auto bS = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"bS"))->get();
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
                        Sprite sprite = _sprite_util->load_sprite(back, x, y, flip);

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
                    auto animatedsprite = _sprite_util->load_animated_sprite(_map_node->find_from_path(url), x, y, flip);
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
                          { return a._id + a._front * 1024 < b._id + b._front * 1024; });
        return v_backgrd;
    }

    std::vector<Portal> MapUtil::load_portal(int mapId)
    {
        return load_portal(load_node(mapId));
    }

    std::vector<Portal> MapUtil::load_portal(wz::Node *node)
    {

        std::vector<Portal> v_portal;

        node = node->get_child(u"portal");
        if (node != nullptr)
        {
            constexpr const char16_t *pt_list[] = {u"sp", u"pi", u"pv", u"pc", u"pg", u"tp", u"ps",
                                                   u"pgi", u"psi", u"pcs", u"ph", u"psh", u"pcj",
                                                   u"pci", u"pcig", u"pshg"};
            for (auto it : node->get_children())
            {
                if (it.second[0]->get_child(u"pt") != nullptr)
                {
                    auto pt = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"pt"))->get();
                    auto tm = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"tm"))->get();

                    if (pt < 0 || pt >= sizeof(pt_list))
                    {
                        continue;
                    }
                    else
                    {
                        auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
                        auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"y"))->get();
                        {
                            auto url = u"MapHelper.img/portal/editor/" + std::basic_string<char16_t>(pt_list[pt]);

                            auto pn = _map_node->find_from_path(url);

                            Sprite sprite = _sprite_util->load_sprite(pn, x, y);

                            Portal portal(sprite, Portal::Type::EDITOR, tm, url);

                            v_portal.push_back(portal);
                        }

                        {
                            if (pt == 7)
                            {
                                pt = 2;
                            }
                            auto url = u"MapHelper.img/portal/game/" + std::basic_string<char16_t>(pt_list[pt]);
                            if (_map_node->find_from_path(url) != NULL)
                            {
                                if (_map_node->find_from_path(url + u"/default") != NULL)
                                {
                                    // 三段式传送门
                                }
                                else
                                {
                                    // 普通的传送门,通常为pv
                                    auto animatedsprite = _sprite_util->load_animated_sprite(_map_node->find_from_path(url), x, y);

                                    Portal portal(animatedsprite, Portal::Type::GAME, tm, url);
                                    v_portal.push_back(portal);
                                }
                            }
                        }
                    }
                }
            }
        }
        return v_portal;
    }

    std::unordered_map<int, FootHold> MapUtil::load_foothold(int mapId)
    {
        return load_foothold(load_node(mapId));
    }

    std::unordered_map<int, FootHold> MapUtil::load_foothold(wz::Node *node)
    {

        std::unordered_map<int, FootHold> m_foothold;

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

                        m_foothold.emplace(id, f);
                    }
                }
            }
        }
        return m_foothold;
    }

    std::unordered_map<int, LadderRope> MapUtil::load_ladderRope(int mapId)
    {
        return load_ladderRope(load_node(mapId));
    }

    std::unordered_map<int, LadderRope> MapUtil::load_ladderRope(wz::Node *node)
    {

        std::unordered_map<int, LadderRope> m_ladderrope;

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

                m_ladderrope.emplace(id, r);
            }
        }
        return m_ladderrope;
    }

    std::optional<Sprite> MapUtil::load_minimap(int mapId)
    {
        std::optional<Sprite> optional;

        auto node = load_node(mapId);
        auto minimap = node->find_from_path(u"miniMap/canvas");
        if (minimap != nullptr)
        {
            optional = _sprite_util->load_sprite(minimap);
        }
        return optional;
    }

    std::optional<Sprite> MapUtil::load_mark(int mapId)
    {
        std::optional<Sprite> optional;

        auto node = load_node(mapId);
        node = node->find_from_path(u"info/mapMark");
        if (node != nullptr)
        {
            auto url = u"MapHelper.img/mark/" + dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
            auto mark = _map_node->find_from_path(url);
            if (mark != NULL)
            {
                optional = _sprite_util->load_sprite(mark);
            }
        }
        return optional;
    }

    wz::Node *MapUtil::load_node(int mapId)
    {
        auto s = std::to_string(mapId);
        if (s.size() < 9)
        {
            s.insert(0, 9 - s.size(), '0');
        }
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + s + ".img";
        return _map_node->find_from_path(path);
    }
}
