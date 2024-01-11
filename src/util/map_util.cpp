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
        auto node = root->find_from_path(StringUtil::to_ustring(path));
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
        node = node->get_child(StringUtil::to_ustring(std::to_string(i)));
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

                SDL_Rect *rect = new SDL_Rect{x - ox, y - oy, width, height};
                Tile t(texture, rect, SDL_PIXELFORMAT_ARGB4444, i, z);

                tile.push_back(t);
            }
        }
        std::sort(tile.begin(), tile.end(), [](const Tile t1, const Tile t2)
                  { return t1._z < t2._z; });
        return tile;
    }

    std::array<std::vector<Obj>, 8> MapUtil::load_obj(int mapId)
    {
        wz::Node *root = WzUtil::current()->Map->get_root();
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + StringUtil::extend_id(mapId, 9) + ".img";
        auto node = root->find_from_path(StringUtil::to_ustring(path));
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
        node = node->get_child(StringUtil::to_ustring(std::to_string(i)))->get_child(u"obj");
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
            Obj o(v_texture, v_rect, v_delay, v_format, i, z, url);
            obj.push_back(o);
        }
        std::sort(obj.begin(), obj.end(), [](const Obj o1, const Obj o2)
                  { return o1._z < o2._z; });
        return obj;
    }

}
