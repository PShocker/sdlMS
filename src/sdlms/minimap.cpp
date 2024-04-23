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
        auto _minimap = Sprite::load_sprite(minimap);

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

        int width = std::max({(int)_minimap._rect.w, (int)_streetName._rect.w + (int)t_offset.x, (int)_mapName._rect.w + (int)t_offset.x});
        int height = _minimap._rect.h;

        auto node = _ui_node->find_from_path(u"UIWindow.img/MiniMap/MaxMap");
        _frame = Frame(node);

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

void MiniMap::draw()
{
    _frame.draw_static({36, 72}, _mask.rect().w, 72);
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