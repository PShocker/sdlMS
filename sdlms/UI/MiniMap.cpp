#include "MiniMap.h"
#include "Core/Core.h"
#include "Resources/Wz.h"
#include "Button.h"
#include "WorldMap.h"
#include "Cursor.h"
#include "ToolTip.h"
#include "Commons/Commons.h"

void MiniMap::init()
{
    auto node = Wz::Map->get_root()->find_from_path(u"MapHelper.img/minimap");
    MiniMap::user = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"user")));
    MiniMap::npc = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"npc")));
    MiniMap::portal = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"portal")));
}

void MiniMap::run()
{
    // NPC,PORTAL,USER
    if (!MiniMap::minimize)
    {
        MiniMap::points.clear();
        const auto load_points = [](SDL_FPoint &p, SDL_Texture *texture, entt::entity ent, int offset_x, int offset_y)
        {
            SDL_FPoint position = p + MiniMap::center;
            position.x = position.x / MiniMap::scale;
            position.y = position.y / MiniMap::scale;

            position.x = MiniMap::x + (MiniMap::backgrnd->w - MiniMap::canvas->w) / 2 + position.x + offset_x;
            position.y = MiniMap::y + 72 + position.y + offset_y;
            SDL_FRect pos_rect = {
                position.x,
                position.y,
                (float)texture->w,
                (float)texture->h};
            MiniMap::points.push_back({pos_rect, texture, ent});
        };
        for (auto ent : World::registry->view<Npc>())
        {
            load_points(World::registry->try_get<Transform>(ent)->position, MiniMap::npc, ent, -2, -4);
        }
        for (auto ent : World::registry->view<Portal>())
        {
            auto por = World::registry->try_get<Portal>(ent);
            // 只渲染普通传送门
            if (por->a.size() == 1)
            {
                load_points(World::registry->try_get<Transform>(ent)->position, MiniMap::portal, ent, -2, -6);
            }
        }
        load_points(World::registry->try_get<Transform>(Player::ent)->position, MiniMap::user, Player::ent, -2, -4);
    }
}

void MiniMap::show()
{
    load_canvas();
    MiniMap::minimize = MiniMap::canvas == nullptr ? true : false;
    load_mapremark();
    load_backgrnd();

    Button::load(u"UIWindow.img/MiniMap/BtMap", BtMap);
    Button::load(u"Basic.img/BtMin2", BtMin2);
}

void MiniMap::hide()
{
    MiniMap::minimize = true;
}

void MiniMap::click()
{
    for (auto &[key, val] : MiniMap::position_map)
    {
        auto rect = val;
        rect.x += MiniMap::x + MiniMap::backgrnd->w;
        rect.y += MiniMap::y;
        Button::click(rect, *key, MiniMap::click_map);
    }
}

void MiniMap::over()
{
    for (auto &[key, val] : MiniMap::position_map)
    {
        auto rect = val;
        rect.x += MiniMap::x + MiniMap::backgrnd->w;
        rect.y += MiniMap::y;
        Button::over(rect, *key);
    }
    for (auto &[r, texture, ent] : MiniMap::points)
    {
        // 判断鼠标是否移动到点附近
        SDL_FPoint point = {Cursor::x, Cursor::y};
        SDL_FRect rect{r.x - 2, r.y - 3, (float)texture->w + 4, (float)texture->h + 6};
        if (SDL_PointInRectFloat(&point, &rect))
        {
            if (auto nametag = World::registry->try_get<NameTag>(ent))
            {
                auto texture = nametag->nametags[0].str_texture;
                SDL_FRect pos_rect = {Cursor::x, Cursor::y, (float)texture->w, (float)texture->h};
                ToolTip::push(pos_rect, texture);
            }
        }
    }
}

bool MiniMap::mousein()
{
    SDL_FPoint point = {Cursor::x, Cursor::y};
    SDL_FRect rect;
    rect.x = MiniMap::x;
    rect.y = MiniMap::y;
    rect.w = MiniMap::backgrnd->w;
    if (MiniMap::minimize)
    {
        rect.h = 24;
    }
    else
    {
        rect.h = MiniMap::backgrnd->h;
    }

    if (SDL_PointInRectFloat(&point, &rect))
    {
        return true;
    }
    return false;
}

void MiniMap::load_canvas()
{
    auto node = Map::load_map_node(Map::id);
    if (node->get_child(u"miniMap"))
    {
        MiniMap::width = dynamic_cast<wz::Property<int> *>(node->find_from_path(u"miniMap/width"))->get();
        MiniMap::height = dynamic_cast<wz::Property<int> *>(node->find_from_path(u"miniMap/height"))->get();
        MiniMap::canvas = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"miniMap/canvas")));
        MiniMap::scale = std::pow(2, dynamic_cast<wz::Property<int> *>(node->find_from_path(u"miniMap/mag"))->get());
        MiniMap::center = SDL_FPoint{(float)dynamic_cast<wz::Property<int> *>(node->find_from_path(u"miniMap/centerX"))->get(),
                                     (float)dynamic_cast<wz::Property<int> *>(node->find_from_path(u"miniMap/centerY"))->get()};
    }
    else
    {
        MiniMap::canvas = nullptr;
    }
}

void MiniMap::load_backgrnd()
{
    if (MiniMap::backgrnd)
    {
        SDL_DestroyTexture(MiniMap::backgrnd);
    }
    int width;
    int height;
    // 获取标题
    auto streetName = FreeType::load(Map::load_streetname(Map::id),
                                     SDL_Color{255, 255, 255, 255}, 0, 15);
    auto streetName_fade = FreeType::load(Map::load_streetname(Map::id),
                                          SDL_Color{0, 0, 0, 255}, 0, 15);
    auto mapName = FreeType::load(Map::load_mapname(Map::id),
                                  SDL_Color{255, 255, 255, 255}, 0, 15);
    auto mapName_fade = FreeType::load(Map::load_mapname(Map::id),
                                       SDL_Color{0, 0, 0, 255}, 0, 15);
    if (canvas)
    {
        width = canvas->w + 12;
        height = canvas->h + 87;
        auto max_str_width = std::max(streetName->w, mapName->w);
        if (max_str_width + 54 > width)
        {
            width = max_str_width + 54;
        }
    }
    else
    {
        width = 120;
        height = 38;
    }
    auto node = Wz::UI->get_root()->find_from_path(u"UIWindow.img/MiniMap");
    auto title = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"title")));
    node = node->find_from_path(u"MaxMap");
    auto c = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"c")));
    auto e = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"e")));
    auto ne = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"ne")));
    auto n = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"n")));
    auto nw = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"nw")));
    auto w = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"w")));
    auto sw = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"sw")));
    auto s = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"s")));
    auto se = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"se")));

    auto texture = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB4444, width, height);
    SDL_SetRenderTarget(Window::renderer, texture);
    SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 0);
    SDL_FRect pos_rect;
    // 先以c填充整个texture,然后在填充四周的边框,最后是四个边角
    pos_rect = {(float)0, (float)0, (float)width, (float)height};
    SDL_RenderTexture(Window::renderer, c, nullptr, &pos_rect);
    // 竖着的两边
    pos_rect = {(float)0, (float)0, (float)w->w, (float)height};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, w, nullptr, &pos_rect);

    pos_rect = {(float)width - e->w, (float)0, (float)e->w, (float)height};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, e, nullptr, &pos_rect);
    // 横着的两边
    pos_rect = {(float)0, 0, (float)width, (float)n->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, n, nullptr, &pos_rect);

    pos_rect = {(float)0, (float)height - s->h, (float)width, (float)s->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, s, nullptr, &pos_rect);
    // 左上
    pos_rect = {0, 0, (float)nw->w, (float)nw->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, nw, nullptr, &pos_rect);
    // 左下
    pos_rect = {0, (float)height - sw->h, (float)sw->w, (float)sw->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, sw, nullptr, &pos_rect);
    // 右上
    pos_rect = {(float)width - ne->w, 0, (float)ne->w, (float)ne->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, ne, nullptr, &pos_rect);
    // 右下
    pos_rect = {(float)width - se->w, (float)height - se->h, (float)se->w, (float)se->h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_RenderTexture(Window::renderer, se, nullptr, &pos_rect);
    // 标题
    pos_rect.x = 6;
    pos_rect.y = 5;
    pos_rect.w = title->w;
    pos_rect.h = title->h;
    SDL_RenderTexture(Window::renderer, title, nullptr, &pos_rect);
    // 地图图标
    pos_rect = {(float)6, (float)24, (float)MiniMap::mapremark->w, (float)MiniMap::mapremark->h};
    SDL_RenderTexture(Window::renderer, MiniMap::mapremark, nullptr, &pos_rect);
    // 街道名称
    pos_rect = {(float)49, (float)25, (float)streetName->w, (float)streetName->h};
    SDL_RenderTexture(Window::renderer, streetName_fade, nullptr, &pos_rect);

    pos_rect = {(float)48, (float)24, (float)streetName->w, (float)streetName->h};
    SDL_RenderTexture(Window::renderer, streetName, nullptr, &pos_rect);
    // 地图名称
    pos_rect = {(float)49, (float)25 + streetName->h, (float)mapName->w, (float)mapName->h};
    SDL_RenderTexture(Window::renderer, mapName_fade, nullptr, &pos_rect);

    pos_rect = {(float)48, (float)24 + streetName->h, (float)mapName->w, (float)mapName->h};
    SDL_RenderTexture(Window::renderer, mapName, nullptr, &pos_rect);

    SDL_SetRenderTarget(Window::renderer, nullptr);

    SDL_DestroyTexture(streetName);
    SDL_DestroyTexture(streetName_fade);
    SDL_DestroyTexture(mapName);
    SDL_DestroyTexture(mapName_fade);

    MiniMap::backgrnd = texture;
}

void MiniMap::load_mapremark()
{
    auto node = Map::load_map_node(Map::id);
    if (node->get_child(u"info"))
    {
        if (auto remark = node->find_from_path(u"info/mapMark"))
        {
            auto remark_str = dynamic_cast<wz::Property<wz::wzstring> *>(remark)->get();
            if (remark_str != u"None")
            {
                MiniMap::mapremark = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(Wz::Map->get_root()->find_from_path(u"MapHelper.img/mark/" + remark_str)));
            }
        }
    }
}

void MiniMap::BtMap_func()
{
    WorldMap::show();
}

void MiniMap::BtMin2_func()
{
    if (MiniMap::canvas)
    {
        MiniMap::minimize = !MiniMap::minimize;
    }
}
