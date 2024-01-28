#include "util/map_util.hpp"
#include "util/wz_util.hpp"
#include "util/string_util.hpp"
#include "sdlms/graphics.hpp"
#include <SDL2/SDL.h>
#include <string>

namespace util
{
    MapUtil::MapUtil()
    {
        _renderer = Graphics::current()->getRenderer();
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
        wz::Node *root = WzUtil::current()->Map->get_root();

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

                auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(root->find_from_path(url));
                auto raw_data = canvas->get_raw_data();
                auto height = canvas->get().height;
                auto width = canvas->get().width;

                auto format = canvas->get().format;

                auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(root->find_from_path(url)->get_child(u"origin"));
                auto ox = o->get().x;
                auto oy = o->get().y;

                auto z = dynamic_cast<wz::Property<int> *>(root->find_from_path(url)->get_child(u"z"))->get();

                SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

                Sprite sprite(raw_data, rect, (int)format);

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
        wz::Node *root = WzUtil::current()->Map->get_root();

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

            auto filp = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"f"))->get();

            for (auto it : root->find_from_path(url)->get_children())
            {
                wz::Property<wz::WzCanvas> *canvas;
                if (it.second[0]->type == wz::Type::UOL)
                {
                    auto i = dynamic_cast<wz::Property<wz::WzUOL> *>(it.second[0]);
                    canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(root->find_from_path(url)->get_child(i->get().uol));
                }
                else if (it.second[0]->type == wz::Type::Canvas)
                {
                    canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(it.second[0]);
                }
                else
                {
                    continue;
                }
                auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
                auto ox = o->get().x;
                auto oy = o->get().y;

                auto delay = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"delay"));
                v_delay.push_back(delay == nullptr ? 0 : delay->get());

                auto a0 = 255;
                auto a1 = 255;

                if (canvas->get_child(u"a0") != NULL && canvas->get_child(u"a1") != NULL)
                {
                    a0 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a0"))->get();
                    a1 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a1"))->get();
                }

                v_a.push_back(std::tuple<int, int>(a0, a1));

                auto raw_data = canvas->get_raw_data();
                auto height = canvas->get().height;
                auto width = canvas->get().width;

                auto format = canvas->get().format;
                SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

                Sprite sprite(raw_data, rect, (int)format, filp);

                v_sprite.push_back(sprite);
            }
            Obj o(v_sprite, v_delay, i, z, url, v_sprite.size(), v_a);
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
        wz::Node *root = WzUtil::current()->Map->get_root();

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
                auto filp = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"f"))->get();

                switch (ani)
                {
                case 0:
                {
                    auto no_str = std::to_string(no);
                    auto url = u"Back/" + bS + u".img/" + u"back" + u"/" + std::u16string{no_str.begin(), no_str.end()};
                    auto back = root->find_from_path(url);
                    if (back != nullptr)
                    {
                        auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(back);
                        auto height = canvas->get().height;
                        auto width = canvas->get().width;
                        auto raw_data = canvas->get_raw_data();

                        auto format = canvas->get().format;

                        auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
                        auto ox = o->get().x;
                        auto oy = o->get().y;

                        cx = cx == 0 ? width : cx;
                        cy = cy == 0 ? height : cy;

                        SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

                        Sprite sprite(raw_data, rect, (int)format, filp);

                        BackGrd backgrd(sprite, id, type, front, rx, ry, cx, cy, ani, url);

                        v_backgrd.push_back(backgrd);
                        break;
                    }
                }
                case 1:
                {
                    auto no_str = std::to_string(no);
                    auto url = u"Back/" + bS + u".img/" + u"ani" + u"/" + std::u16string{no_str.begin(), no_str.end()};
                    std::vector<Sprite> v_sprite;
                    std::vector<int> v_delay;
                    std::vector<std::tuple<int, int>> v_a;
                    for (auto it : root->find_from_path(url)->get_children())
                    {
                        wz::Property<wz::WzCanvas> *canvas;
                        if (it.second[0]->type == wz::Type::UOL)
                        {
                            auto i = dynamic_cast<wz::Property<wz::WzUOL> *>(it.second[0]);
                            canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(root->find_from_path(url)->get_child(i->get().uol));
                        }
                        else if (it.second[0]->type == wz::Type::Canvas)
                        {
                            canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(it.second[0]);
                        }
                        else
                        {
                            continue;
                        }
                        auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
                        auto ox = o->get().x;
                        auto oy = o->get().y;

                        auto delay = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"delay"));
                        v_delay.push_back(delay == nullptr ? 0 : delay->get());

                        auto a0 = 255;
                        auto a1 = 255;
                        if (canvas->get_child(u"a0") != NULL && canvas->get_child(u"a1") != NULL)
                        {
                            a0 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a0"))->get();
                            a1 = dynamic_cast<wz::Property<int> *>(canvas->get_child(u"a1"))->get();
                        }
                        v_a.push_back(std::tuple<int, int>(a0, a1));

                        auto raw_data = canvas->get_raw_data();
                        auto height = canvas->get().height;
                        auto width = canvas->get().width;

                        auto format = canvas->get().format;
                        SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

                        Sprite sprite(raw_data, rect, (int)format, filp);
                        v_sprite.push_back(sprite);
                    }
                    AnimatedSprite animatedsprite(v_sprite, v_delay, v_sprite.size(), v_a);
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
        wz::Node *root = WzUtil::current()->Map->get_root();

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

                            auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(root->find_from_path(url));
                            auto raw_data = canvas->get_raw_data();
                            auto height = canvas->get().height;
                            auto width = canvas->get().width;

                            auto format = canvas->get().format;

                            auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
                            auto ox = o->get().x;
                            auto oy = o->get().y;

                            SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

                            Sprite sprite(raw_data, rect, (int)format);

                            Portal portal(sprite, Portal::Type::EDITOR, tm, url);

                            v_portal.push_back(portal);
                        }

                        {
                            if (pt == 7)
                            {
                                pt = 2;
                            }
                            auto url = u"MapHelper.img/portal/game/" + std::basic_string<char16_t>(pt_list[pt]);
                            if (root->find_from_path(url) != NULL)
                            {
                                if (root->find_from_path(url + u"/default") != NULL)
                                {
                                    // 三段式传送门
                                }
                                else
                                {
                                    // 普通的传送门,通常为pv
                                    std::vector<Sprite> v_sprite;
                                    std::vector<int> v_delay;
                                    std::vector<std::tuple<int, int>> v_a;

                                    for (auto it : root->find_from_path(url)->get_children())
                                    {
                                        auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(it.second[0]);
                                        auto height = canvas->get().height;
                                        auto width = canvas->get().width;
                                        auto raw_data = canvas->get_raw_data();

                                        auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
                                        auto ox = o->get().x;
                                        auto oy = o->get().y;

                                        auto format = canvas->get().format;

                                        SDL_FRect rect{(float)x - ox, (float)y - oy, (float)width, (float)height};

                                        Sprite sprite(raw_data, rect, (int)format);

                                        v_delay.push_back(100);

                                        v_sprite.push_back(sprite);

                                        auto a0 = 255;
                                        auto a1 = 255;

                                        v_a.push_back(std::tuple<int, int>(a0, a1));
                                    }
                                    AnimatedSprite animatedsprite(v_sprite, v_delay, v_sprite.size(), v_a);
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

    Sprite *MapUtil::load_minimap(int mapId)
    {
        auto node = load_node(mapId);
        auto minimap = node->find_from_path(u"miniMap/canvas");
        if (minimap != nullptr)
        {
            auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(minimap);
            auto height = canvas->get().height;
            auto width = canvas->get().width;
            auto raw_data = canvas->get_raw_data();

            SDL_FRect rect{0, 0, (float)width, (float)height};

            auto format = canvas->get().format;

            return new Sprite(raw_data, rect, (int)format);
        }
        else
        {
            return nullptr;
        }
    }

    Sprite *MapUtil::load_mark(int mapId)
    {
        auto node = load_node(mapId);
        node = node->find_from_path(u"info/mapMark");
        if (node != nullptr)
        {
            auto url = u"MapHelper.img/mark/" + dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
            wz::Node *root = WzUtil::current()->Map->get_root();
            auto mark = root->find_from_path(url);
            if (mark != NULL)
            {
                auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(mark);
                auto height = canvas->get().height;
                auto width = canvas->get().width;
                auto raw_data = canvas->get_raw_data();

                SDL_FRect rect{0, 0, (float)width, (float)height};

                auto format = canvas->get().format;

                return new Sprite(raw_data, rect, (int)format);
            }
        }
        else
        {
            return nullptr;
        }
        return nullptr;
    }

    wz::Node *MapUtil::load_node(int mapId)
    {
        wz::Node *root = WzUtil::current()->Map->get_root();
        auto s = std::to_string(mapId);
        if (s.size() < 9)
        {
            s.insert(0, 9 - s.size(), '0');
        }
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + s + ".img";
        return root->find_from_path(path);
    }
}
