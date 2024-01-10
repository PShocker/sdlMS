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

    std::vector<Tile> MapUtil::load_tile(int mapId)
    {
        wz::Node *root = WzUtil::current()->Map->get_root();
        std::string path = "Map/Map" + std::to_string(mapId / 100000000) + "/" + StringUtil::extend_id(mapId, 9) + ".img";
        auto node = root->find_from_path(StringUtil::to_ustring(path));
        std::vector<Tile> tile;
        for (size_t i = 0; i < 8; i++)
        {
            load_tile(root, node, i, tile);
        }
        return tile;
    }

    void MapUtil::load_tile(wz::Node *root, wz::Node *node, int i, std::vector<Tile> &tile)
    {
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
    }
}
