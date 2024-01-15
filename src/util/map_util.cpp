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
        wz::Node *root = WzUtil::current()->Map->get_root();
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + StringUtil::extend_id(mapId, 9) + ".img";
        auto node = root->find_from_path(path);
        std::array<std::vector<Tile>, 8> tile;
        for (size_t i = 0; i < 8; i++)
        {
            tile[i] = load_tile(root, node, i);
        }
        return tile;
    }

    std::vector<Tile> MapUtil::load_tile(wz::Node *root, wz::Node *node, int i)
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

                auto url = u"Tile/" + dynamic_cast<wz::Property<wz::wzstring> *>(tS)->get() + u".img/" + u + u"/" + StringUtil::to_ustring(std::to_string(no));

                auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(root->find_from_path(url));
                auto raw_data = canvas->get_raw_data();
                auto height = canvas->get().height;
                auto width = canvas->get().width;

                auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(root->find_from_path(url)->get_child(u"origin"));
                auto ox = o->get().x;
                auto oy = o->get().y;

                auto z = dynamic_cast<wz::Property<int> *>(root->find_from_path(url)->get_child(u"z"))->get();
                if (z == 0)
                {
                    z = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"zM"))->get();
                }
                SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, width, height);
                SDL_UpdateTexture(texture, NULL, raw_data.data(), width * sizeof(Uint16));
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                SDL_FRect *rect = new SDL_FRect{(float)x - ox, (float)y - oy, (float)width, (float)height};
                Tile t(texture, rect, SDL_PIXELFORMAT_ARGB4444, i, z);

                tile.push_back(t);
            }
        }
        std::ranges::sort(tile, [](const Tile a, const Tile b)
                          { return a._z < b._z; });
        return tile;
    }

    std::array<std::vector<Obj>, 8> MapUtil::load_obj(int mapId)
    {
        wz::Node *root = WzUtil::current()->Map->get_root();
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + StringUtil::extend_id(mapId, 9) + ".img";
        auto node = root->find_from_path(path);
        std::array<std::vector<Obj>, 8> obj;
        for (size_t i = 0; i < 8; i++)
        {
            obj[i] = load_obj(root, node, i);
        }
        return obj;
    }

    std::vector<Obj> MapUtil::load_obj(wz::Node *root, wz::Node *node, int i)
    {
        std::vector<Obj> obj;
        node = node->get_child(std::to_string(i))->get_child(u"obj");
        for (auto it : node->get_children())
        {
            std::vector<SDL_Texture *> v_texture;
            std::vector<SDL_Rect *> v_rect;
            std::vector<int> v_delay;
            std::vector<int> v_format;
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

                auto raw_data = canvas->get_raw_data();
                auto height = canvas->get().height;
                auto width = canvas->get().width;

                SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, width, height);
                SDL_UpdateTexture(texture, NULL, raw_data.data(), width * sizeof(Uint16));
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
                v_texture.push_back(texture);

                SDL_Rect *rect = new SDL_Rect{x - ox, y - oy, width, height};
                v_rect.push_back(rect);
            }
            Obj o(v_texture, v_rect, v_delay, v_format, i, z, filp, url, v_texture.size());
            obj.push_back(o);
        }
        std::ranges::sort(obj, [](const Obj a, const Obj b)
                          { return a._z < b._z; });
        return obj;
    }

    std::vector<BackGrd> MapUtil::load_backgrd(int mapId)
    {
        wz::Node *root = WzUtil::current()->Map->get_root();
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + StringUtil::extend_id(mapId, 9) + ".img";
        auto node = root->find_from_path(path);
        return load_backgrd(root, node);
    }

    std::vector<BackGrd> MapUtil::load_backgrd(wz::Node *root, wz::Node *node)
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

                auto id = std::stoi(StringUtil::to_string(it.first));
                // auto id = 0;

                // std::vector<SDL_Texture *> v_texture;
                // std::vector<SDL_Rect *> v_rect;
                // std::vector<int> v_delay;
                // std::vector<int> v_format;

                switch (ani)
                {
                case 0:
                {
                    auto url = u"Back/" + bS + u".img/" + u"back" + u"/" + StringUtil::to_ustring(std::to_string(no));
                    auto back = root->find_from_path(url);
                    auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(back);
                    auto height = canvas->get().height;
                    auto width = canvas->get().width;
                    auto raw_data = canvas->get_raw_data();

                    auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
                    auto ox = o->get().x;
                    auto oy = o->get().y;

                    cx = cx == 0 ? width : cx;
                    cy = cy == 0 ? height : cy;

                    SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, width, height);
                    SDL_UpdateTexture(texture, NULL, raw_data.data(), width * sizeof(Uint16));
                    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                    SDL_FRect *rect = new SDL_FRect{(float)x - ox, (float)y - oy, (float)width, (float)height};

                    Sprite sprite(texture, rect, SDL_PIXELFORMAT_ARGB4444);

                    BackGrd backgrd(sprite, id, type, front, rx, ry, cx, cy, ani, url);

                    v_backgrd.push_back(backgrd);
                    break;
                }
                case 1:
                {
                    auto url = u"Back/" + bS + u".img/" + u"ani" + u"/" + StringUtil::to_ustring(std::to_string(no));
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
        wz::Node *root = WzUtil::current()->Map->get_root();
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + StringUtil::extend_id(mapId, 9) + ".img";
        auto node = root->find_from_path(path);
        return load_portal(root, node);
    }

    std::vector<Portal> MapUtil::load_portal(wz::Node *root, wz::Node *node)
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

                            auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
                            auto ox = o->get().x;
                            auto oy = o->get().y;

                            SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, width, height);
                            SDL_UpdateTexture(texture, NULL, raw_data.data(), width * sizeof(Uint16));
                            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                            SDL_FRect *rect = new SDL_FRect{(float)x - ox, (float)y - oy, (float)width, (float)height};

                            Sprite sprite(texture, rect, SDL_PIXELFORMAT_ARGB4444);

                            Portal portal(sprite, 0, url);

                            v_portal.push_back(portal);
                        }

                        {
                            if (pt == 7)
                            {
                                pt = 2;
                            }
                            auto url = u"MapHelper.img/portal/game/" + std::basic_string<char16_t>(pt_list[pt]);
                            if (root->find_from_path(url) != nullptr)
                            {
                                // auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(root->find_from_path(url));
                                // auto raw_data = canvas->get_raw_data();
                                // auto height = canvas->get().height;
                                // auto width = canvas->get().width;
                            }
                        }
                    }
                }
            }
        }
        return v_portal;
    }
}
