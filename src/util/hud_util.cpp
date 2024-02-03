#include "util/hud_util.hpp"
#include "util/wz_util.hpp"

#include "sdlms/graphics.hpp"

#include "wz/Property.hpp"

#include "sdlms/graphics.hpp"

namespace util
{
    HudUtil::HudUtil()
    {
        _renderer = Graphics::current()->getRenderer();
        _sprite_util = SpriteUtil::current();
        _freetype = FreeType::current();
        _string_util = StringUtil::current();
        _map_util = MapUtil::current();

        _ui_node = WzUtil::current()->UI->get_root();
    }

    std::vector<Sprite> HudUtil::load_minimap(int mapId)
    {
        std::vector<Sprite> v_s;

        auto o_minimap = _map_util->load_minimap(mapId);
        if (o_minimap.has_value() == true)
        {
            auto minimap = o_minimap.value();
            auto [mapName, streetName] = _string_util->load_map_name(mapId);

            SDL_Point t_offset = {52, 25};

            auto t_up = _freetype->load_str(streetName);

            auto t_down = _freetype->load_str(mapName);

            int width = std::max((int)minimap._rect.w, std::max((int)std::get<1>(t_up) + (int)t_offset.x, (int)std::get<1>(t_down) + (int)t_offset.x));
            int height = minimap._rect.h;

            auto node = _ui_node->find_from_path(u"UIWindow.img/MiniMap/MaxMap");
            auto nw = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")));
            v_s.push_back(nw);

            auto ne = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")));
            ne._rect.x = width + 6;
            v_s.push_back(ne);

            auto sw = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")));
            sw._rect.y = height + 72;
            v_s.push_back(sw);

            auto se = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")));
            se._rect.x = width + 6;
            se._rect.y = height + 72;
            v_s.push_back(se);

            auto n = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")));

            for (size_t x = 6; x <= width + 6; x++)
            {
                SDL_FRect rect{(float)x, n._rect.y, n._rect.w, n._rect.h};
                v_s.push_back(Sprite(n._texture, rect, n._flip));
            }

            auto s = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")));

            s._rect.x = 6;
            s._rect.y = height + 72;

            for (size_t x = 6; x <= width + 6; x++)
            {
                SDL_FRect rect{(float)x, s._rect.y, s._rect.w, s._rect.h};
                v_s.push_back(Sprite(s._texture, rect, s._flip));
            }

            auto w = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")));

            w._rect.x = 0;
            w._rect.y = 0;

            for (size_t y = 72; y < height + 72; y++)
            {
                SDL_FRect rect{w._rect.x, (float)y, w._rect.w, w._rect.h};
                v_s.push_back(Sprite(w._texture, rect, w._flip));
            }

            auto e = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")));

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

            v_s.push_back(Sprite(std::get<0>(t_up), SDL_FRect{(float)t_offset.x, (float)t_offset.y, (float)std::get<1>(t_up), (float)std::get<2>(t_up)}, SDL_FLIP_NONE));

            v_s.push_back(Sprite(std::get<0>(t_down), SDL_FRect{(float)t_offset.x, (float)t_offset.y + (float)std::get<2>(t_up), (float)std::get<1>(t_down), (float)std::get<2>(t_down)}, SDL_FLIP_NONE));

            auto o_mark = _map_util->load_mark(mapId);
            if (o_mark.has_value())
            {
                auto mark = o_mark.value();
                mark._rect.x = 10;
                mark._rect.y = 22;

                v_s.push_back(mark);
            }

            auto title = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_parent()->find_from_path(u"title")));
            title._rect.x = 7;
            title._rect.y = 5;

            v_s.push_back(title);
        }
        return v_s;
    }

    std::vector<Sprite> HudUtil::load_statusbar()
    {
        std::vector<Sprite> v_s;

        auto ui = _ui_node->find_from_path(u"StatusBar.img");

        auto backgrnd = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui->find_from_path(u"base/backgrnd")));

        backgrnd._rect.y = Graphics::SCREEN_HEIGHT - backgrnd._rect.h;

        for (size_t i = 0; i < Graphics::SCREEN_WIDTH; i += backgrnd._rect.w)
        {
            SDL_FRect rect{i, backgrnd._rect.y, backgrnd._rect.w, backgrnd._rect.h};
            v_s.push_back(Sprite(backgrnd._texture, rect, SDL_FLIP_NONE));
        }

        auto backgrnd2 = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui->find_from_path(u"base/backgrnd2")));

        backgrnd2._rect.y = Graphics::SCREEN_HEIGHT - backgrnd2._rect.h;

        v_s.push_back(backgrnd2);

        auto chatTarget = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui->find_from_path(u"base/chatTarget")));

        chatTarget._rect.y = Graphics::SCREEN_HEIGHT - backgrnd2._rect.h + 8;

        v_s.push_back(chatTarget);

        auto quickSlot = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui->find_from_path(u"base/quickSlot")));

        quickSlot._rect.x = Graphics::SCREEN_WIDTH - quickSlot._rect.w;
        quickSlot._rect.y = Graphics::SCREEN_HEIGHT - quickSlot._rect.h;

        v_s.push_back(quickSlot);

        return v_s;
    }

    std::vector<EventSprite *> HudUtil::load_event_sprite()
    {
        std::vector<EventSprite *> v_s;

        auto StatusBar = _ui_node->find_from_path(u"StatusBar.img");

        auto BtShop = _sprite_util->load_event_sprite(StatusBar->find_from_path(u"BtShop"),590,Graphics::SCREEN_HEIGHT-18);

        v_s.push_back(BtShop);

        return v_s;
    }
}
