#include "util/minimap_util.hpp"
#include "util/wz_util.hpp"

#include "sdlms/graphics.hpp"

#include "wz/Property.hpp"

#include "sdlms/graphics.hpp"

namespace util
{
    MiniMapUtil::MiniMapUtil()
    {
        _renderer = Graphics::current()->getRenderer();
        _freetype_util = FreeTypeUtil::current();
        _string_util = StringUtil::current();
        _map_util = MapUtil::current();

        _ui_node = WzUtil::current()->UI->get_root();
    }

    std::vector<Sprite> MiniMapUtil::load_minimap(int mapId)
    {
        std::vector<Sprite> v_s;

        auto o_minimap = _map_util->load_minimap(mapId);
        if (o_minimap.has_value() == true)
        {
            auto minimap = o_minimap.value();
            auto map = _string_util->load_map_info(mapId);

            auto mapName = map[u"mapName"];
            auto streetName = map[u"streetName"];

            SDL_Point t_offset = {52, 25};

            auto t_up = _freetype_util->load_map_str(streetName);

            auto t_down = _freetype_util->load_map_str(mapName);

            int width = std::max({(int)minimap._rect.w, (int)t_up._rect.w + (int)t_offset.x, (int)t_down._rect.w + (int)t_offset.x});
            int height = minimap._rect.h;

            auto node = _ui_node->find_from_path(u"UIWindow.img/MiniMap/MaxMap");
            auto nw = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")));
            v_s.push_back(nw);

            auto ne = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")));
            ne._rect.x = width + 6;
            v_s.push_back(ne);

            auto sw = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")));
            sw._rect.y = height + 72;
            v_s.push_back(sw);

            auto se = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")));
            se._rect.x = width + 6;
            se._rect.y = height + 72;
            v_s.push_back(se);

            auto n = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")));

            for (size_t x = 6; x <= width + 6; x++)
            {
                SDL_FRect rect{(float)x, n._rect.y, n._rect.w, n._rect.h};
                v_s.push_back(Sprite(n._texture, rect, n._flip));
            }

            auto s = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")));

            s._rect.x = 6;
            s._rect.y = height + 72;

            for (size_t x = 6; x <= width + 6; x++)
            {
                SDL_FRect rect{(float)x, s._rect.y, s._rect.w, s._rect.h};
                v_s.push_back(Sprite(s._texture, rect, s._flip));
            }

            auto w = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")));

            w._rect.x = 0;
            w._rect.y = 0;

            for (size_t y = 72; y < height + 72; y++)
            {
                SDL_FRect rect{w._rect.x, (float)y, w._rect.w, w._rect.h};
                v_s.push_back(Sprite(w._texture, rect, w._flip));
            }

            auto e = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")));

            e._rect.x = width + 6;
            e._rect.y = 0;

            for (size_t y = 72; y < height + 72; y++)
            {
                SDL_FRect rect{e._rect.x, (float)y, e._rect.w, e._rect.h};
                v_s.push_back(Sprite(e._texture, rect, e._flip));
            }

            // // 创建需要填充的矩形
            SDL_FRect rect = {6, 72, (float)width, (float)height};

            SDL_Texture *texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_TARGET, rect.w, rect.h);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_SetRenderTarget(_renderer, texture);
            SDL_SetRenderDrawColor(_renderer, 0x00, 0x00, 0x0F, 0x7F); // 使用ARGB4444格式，每个分量占4位
            SDL_RenderFillRect(_renderer, NULL);
            // 将渲染目标重新设置为默认渲染器
            SDL_SetRenderTarget(_renderer, NULL);

            v_s.push_back(Sprite(texture, rect, SDL_FLIP_NONE));

            minimap._rect.x = std::max((float)(width - minimap._rect.w) / 2, (float)6);
            minimap._rect.y = 72;
            v_s.push_back(minimap);

            t_up._rect.x = t_offset.x;
            t_up._rect.y = t_offset.y;
            v_s.push_back(t_up);

            t_down._rect.x = t_offset.x;
            t_down._rect.y = t_offset.y + t_up._rect.h + 4;
            v_s.push_back(t_down);

            auto o_mark = _map_util->load_mark(mapId);
            if (o_mark.has_value())
            {
                auto mark = o_mark.value();
                mark._rect.x = 10;
                mark._rect.y = 22;

                v_s.push_back(mark);
            }

            auto title = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_parent()->find_from_path(u"title")));
            title._rect.x = 7;
            title._rect.y = 5;

            v_s.push_back(title);
        }
        return v_s;
    }
}
