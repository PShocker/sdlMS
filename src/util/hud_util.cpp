#include "util/hud_util.hpp"
#include "util/wz_util.hpp"
#include "util/freetype.hpp"
#include "util/string_util.hpp"

#include "sdlms/graphics.hpp"

#include "wz/Property.hpp"

namespace util
{
    HudUtil::HudUtil()
    {
        _renderer = Graphics::current()->getRenderer();
    }

    std::vector<Sprite> HudUtil::load_minimap(int mapId)
    {
        std::vector<Sprite> v_s;

        auto minimap = MapUtil::current()->load_minimap(mapId);
        if (minimap != nullptr)
        {
            int width = minimap->_rect.w;
            int height = minimap->_rect.h;

            auto node = WzUtil::current()->UI->get_root()->find_from_path(u"UIWindow.img/MiniMap/MaxMap");
            auto func = [](wz::Property<wz::WzCanvas> *canvas) -> Sprite
            {
                auto height = canvas->get().height;
                auto width = canvas->get().width;
                auto raw_data = canvas->get_raw_data();
                auto format = canvas->get().format;

                auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(canvas->get_child(u"origin"));
                auto ox = o->get().x;
                auto oy = o->get().y;

                SDL_FRect rect{(float)ox, (float)oy, (float)width, (float)height};
                Sprite sprite(raw_data, rect, (int)format);
                return sprite;
            };
            auto nw = func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")));
            v_s.push_back(nw);

            auto ne = func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")));
            ne._rect.x = width + 6;
            v_s.push_back(ne);

            auto sw = func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")));
            sw._rect.y = height + 72;
            v_s.push_back(sw);

            auto se = func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")));
            se._rect.x = width + 6;
            se._rect.y = height + 72;
            v_s.push_back(se);

            auto n = func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")));

            for (size_t x = 6; x <= width + 6; x++)
            {
                SDL_FRect rect{(float)x, n._rect.y, n._rect.w, n._rect.h};
                v_s.push_back(Sprite(n._texture, rect, n._flip));
            }

            auto s = func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")));

            s._rect.x = 6;
            s._rect.y = height + 72;

            for (size_t x = 6; x <= width + 6; x++)
            {
                SDL_FRect rect{(float)x, s._rect.y, s._rect.w, s._rect.h};
                v_s.push_back(Sprite(s._texture, rect, s._flip));
            }

            auto w = func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")));

            w._rect.x = 0;
            w._rect.y = 0;

            for (size_t y = 72; y < height + 72; y++)
            {
                SDL_FRect rect{w._rect.x, (float)y, w._rect.w, w._rect.h};
                v_s.push_back(Sprite(w._texture, rect, w._flip));
            }

            auto e = func(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")));

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

            minimap->_rect.x = 6;
            minimap->_rect.y = 72;
            v_s.push_back(*minimap);

            auto [mapName, streetName] = StringUtil::current()->load_map_name(mapId);

            auto t_up = FreeType::current()->load_str(streetName);

            v_s.push_back(Sprite(std::get<0>(t_up), SDL_FRect{52, 22, (float)std::get<1>(t_up), (float)std::get<2>(t_up)}, SDL_FLIP_NONE));

            auto t_down = FreeType::current()->load_str(mapName);

            v_s.push_back(Sprite(std::get<0>(t_down), SDL_FRect{52, 40, (float)std::get<1>(t_down), (float)std::get<2>(t_down)}, SDL_FLIP_NONE));

            auto mark = MapUtil::current()->load_mark(mapId);

            mark->_rect.x = 10;
            mark->_rect.y = 20;

            v_s.push_back(*mark);
        }
        return v_s;
    }
}