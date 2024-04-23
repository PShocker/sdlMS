#include "sdlms/minimap.hpp"
#include "sdlms/foothold.hpp"
#include "util/wz_util.hpp"
#include "util/map_util.hpp"
#include "minimap.hpp"

MiniMap::MiniMap(int mapId)
{
    auto _string_util = util::StringUtil::current();
    auto _freetype_util = util::FreeTypeUtil::current();
    auto _ui_node = util::WzUtil::current()->UI->get_root();
    auto _renderer = Graphics::current()->getRenderer();

    auto node = util::MapUtil::current()->load_map_node(mapId);
    auto minimap = node->find_from_path(u"miniMap/canvas");
    if (minimap != nullptr)
    {
        _canvas = Sprite::load_sprite(minimap);

        auto map = _string_util->load_map_info(mapId);

        auto mapName = map[u"mapName"];
        auto streetName = map[u"streetName"];

        _streetName = _freetype_util->load_map_str(streetName);

        _mapName = _freetype_util->load_map_str(mapName);

        SDL_Point t_offset = {52, 25};

        _streetName._rect.x = t_offset.x;
        _streetName._rect.y = t_offset.y;

        _mapName._rect.x = t_offset.x;
        _mapName._rect.y = t_offset.y + _streetName._rect.h + 4;

        int width = std::max({(int)_canvas._rect.w, (int)_streetName._rect.w + (int)t_offset.x, (int)_mapName._rect.w + (int)t_offset.x});
        int height = _canvas._rect.h;

        _canvas._rect.x = std::max((float)(width - _canvas._rect.w) / 2, (float)6);
        _canvas._rect.y = 72;

        auto node = _ui_node->find_from_path(u"UIWindow.img/MiniMap/MaxMap");

        _nw = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")));
        _ne = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")));
        _ne._rect.x = width + 6;
        _sw = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")));
        _sw._rect.y = height + 72;
        _se = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")));
        _se._rect.x = width + 6;
        _se._rect.y = height + 72;

        auto n = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")));
        for (size_t x = 6; x <= width + 6; x++)
        {
            SDL_FRect rect{(float)x, n._rect.y, n._rect.w, n._rect.h};
            _n.push_back(Sprite(n._texture, rect, n._flip));
        }

        auto s = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")));

        s._rect.x = 6;
        s._rect.y = height + 72;

        for (size_t x = 6; x <= width + 6; x++)
        {
            SDL_FRect rect{(float)x, s._rect.y, s._rect.w, s._rect.h};
            _s.push_back(Sprite(s._texture, rect, s._flip));
        }

        auto w = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")));

        w._rect.x = 0;
        w._rect.y = 0;

        for (size_t y = 72; y < height + 72; y++)
        {
            SDL_FRect rect{w._rect.x, (float)y, w._rect.w, w._rect.h};
            _w.push_back(Sprite(w._texture, rect, w._flip));
        }

        auto e = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")));
        e._rect.x = width + 6;
        e._rect.y = 0;

        for (size_t y = 72; y < height + 72; y++)
        {
            SDL_FRect rect{e._rect.x, (float)y, e._rect.w, e._rect.h};
            _e.push_back(Sprite(e._texture, rect, e._flip));
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

        _mask = Sprite(texture, rect, SDL_FLIP_NONE);

        auto mark = load_minimap_mark(mapId);
        if (mark.has_value())
        {
            _mark = mark.value();
            _mark.value()._rect.x = 10;
            _mark.value()._rect.y = 22;
        }

        _title = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_parent()->find_from_path(u"title")));
        _title._rect.x = 7;
        _title._rect.y = 5;
    }
}

std::optional<Sprite> MiniMap::load_minimap_mark(int mapId)
{
    std::optional<Sprite> optional;

    auto _map_node = util::WzUtil::current()->Map->get_root();

    auto node = util::MapUtil::current()->load_map_node(mapId);
    node = node->find_from_path(u"info/mapMark");
    if (node != nullptr)
    {
        auto url = u"MapHelper.img/mark/" + dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
        auto mark = _map_node->find_from_path(url);
        if (mark != NULL)
        {
            optional = Sprite::load_sprite(mark);
        }
    }
    return optional;
}

void MiniMap::draw()
{
    _nw.draw_static();
    _ne.draw_static();
    _sw.draw_static();
    _se.draw_static();
    for (auto &it : _n)
    {
        it.draw_static();
    }
    for (auto &it : _s)
    {
        it.draw_static();
    }
    for (auto &it : _w)
    {
        it.draw_static();
    }
    for (auto &it : _e)
    {
        it.draw_static();
    }
    _mask.draw_static();
    _canvas.draw_static();
    _streetName.draw_static();
    _mapName.draw_static();
    if (_mark.has_value())
    {
        _mark.value().draw_static();
    }
    _title.draw_static();
}
