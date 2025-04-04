#include "WorldMap.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Systems/UI.h"
#include "Button.h"

void WorldMap::show(std::u16string path)
{
    WorldMap::links.clear();
    WorldMap::spots.clear();
    WorldMap::curpos.p = std::nullopt;

    WorldMap::x = Camera::w / 2;
    WorldMap::y = Camera::h / 2;

    auto node = Wz::Map->get_root()->find_from_path(u"MapHelper.img/worldMap");
    // 默认是WorldMap010
    node = Wz::Map->get_root()->find_from_path(u"WorldMap/" + path + u".img");
    if (node->find_from_path(u"MapLink"))
    {
        for (auto &[key, val] : node->find_from_path(u"MapLink")->get_children())
        {
            MapLink maplink;
            maplink.spr = Sprite(val[0]->find_from_path(u"link/linkImg"));
            maplink.linkMap = dynamic_cast<wz::Property<wz::wzstring> *>(val[0]->find_from_path(u"link/linkMap"))->get();
            WorldMap::links.push_back(maplink);
        }
    }

    WorldMap::baseimg.spr = Sprite(node->find_from_path(u"BaseImg/0"));

    auto mapList = node->find_from_path(u"MapList");
    for (auto &[key, val] : mapList->get_children())
    {
        Spot spt;
        auto spot = val[0]->get_child(u"spot");
        auto type = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"type"))->get();
        auto spot_v = dynamic_cast<wz::Property<wz::WzVec2D> *>(spot)->get();
        spt.p = SDL_FPoint{(float)spot_v.x, (float)spot_v.y};
        std::vector<int> mapNo;
        // 从第0帧顺序读
        for (int i = 0; i < val[0]->get_child(u"mapNo")->children_count(); i++)
        {
            auto it = val[0]->get_child(u"mapNo")->get_child(std::to_string(i));
            if (it == nullptr)
            {
                // 如果发现没读取到,说明已经读完,则退出读取
                break;
            }
            int no = 0;
            if (it->type == wz::Type::Int)
            {
                no = dynamic_cast<wz::Property<int> *>(it)->get();
            }
            else
            {
                auto no_str = dynamic_cast<wz::Property<wz::wzstring> *>(it)->get();
                no = std::stoi(std::string{no_str.begin(), no_str.end()});
            }
            if (no == Map::id)
            {
                WorldMap::curpos.p = spt.p;
                WorldMap::curpos.aspr = AnimatedSprite(Wz::Map->get_root()->find_from_path(u"MapHelper.img/worldMap/curPos"));
            }
            mapNo.push_back(no);
        }
        spt.mapNo = mapNo;
        node = Wz::Map->get_root()->find_from_path(u"MapHelper.img/worldMap");
        switch (type)
        {
        case 0:
        {
            spt.spr = Sprite(node->find_from_path(u"mapImage/0"));
        }
        break;
        case 1:
        {
            spt.spr = Sprite(node->find_from_path(u"mapImage/1"));
        }
        break;
        case 2:
        {
            spt.spr = Sprite(node->find_from_path(u"mapImage/2"));
        }
        break;
        case 3:
        {
            spt.spr = Sprite(node->find_from_path(u"mapImage/3"));
        }
        break;
        default:
            break;
        }
        WorldMap::spots.push_back(spt);
    }
    WorldMap::load_backgrnd();
    Button::load(u"UIWindow.img/KeyConfig/BtClose", BtClose);

    ui_index.push_back(UIIndex::UI_WorldMap);
    WorldMap::open = true;
}

void WorldMap::hide()
{
    ui_index.remove(UIIndex::UI_WorldMap);
    WorldMap::open = false;
}

void WorldMap::click()
{
    if (auto spt = mousein_maplist())
    {
        // switch map
        World::TransPort::id = spt->mapNo[0];
        World::TransPort::tn = u"sp";
        WorldMap::hide();
        return;
    }
    if (WorldMap::cur_link.has_value())
    {
        auto link = WorldMap::cur_link.value();
        WorldMap::hide();
        WorldMap::show(link.linkMap);
        return;
    }

    for (auto &[key, val] : WorldMap::position_map)
    {
        auto rect = val;
        rect.x += WorldMap::x - WorldMap::baseimg.spr.sprw->origin.x - 5 + WorldMap::backgrnd->w;
        rect.y += WorldMap::y - WorldMap::baseimg.spr.sprw->origin.y - 4;
        Button::click(rect, *key, WorldMap::click_map);
    }
}

void WorldMap::over()
{
    if (mousein_maplist())
    {
        Cursor::type = u"1";
    }
    if (auto link = mousein_maplink())
    {
        cur_link = *link;
    }
    else
    {
        cur_link = std::nullopt;
    }
    for (auto &[key, val] : WorldMap::position_map)
    {
        auto rect = val;
        rect.x += WorldMap::x - WorldMap::baseimg.spr.sprw->origin.x - 5 + WorldMap::backgrnd->w;
        rect.y += WorldMap::y - WorldMap::baseimg.spr.sprw->origin.y - 4;
        Button::over(rect, *key);
    }
    return;
}

void WorldMap::run()
{
    if (Input::state[SDL_SCANCODE_W])
    {
        if (keyborard)
        {
            if (!WorldMap::open)
            {
                WorldMap::show();
            }
            else
            {
                WorldMap::hide();
            }
            keyborard = false;
        }
    }
    else
    {
        keyborard = true;
    }
}

bool WorldMap::mousein()
{
    float mouse_x = Window::mouse_x;
    float mouse_y = Window::mouse_y;

    SDL_FPoint point = {mouse_x, mouse_y};
    SDL_FRect rect;
    auto sprw = WorldMap::baseimg.spr.sprw;
    rect.x = WorldMap::x - sprw->origin.x - 5;
    rect.y = WorldMap::y - sprw->origin.y - 4;
    rect.w = WorldMap::backgrnd->w;
    rect.h = WorldMap::backgrnd->h;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        return true;
    }
    return false;
}

WorldMap::Spot *WorldMap::mousein_maplist()
{
    float mouse_x = Window::mouse_x;
    float mouse_y = Window::mouse_y;
    // 判断是否点击到spot
    for (auto &it : WorldMap::spots)
    {
        auto spot = &it;
        auto spot_x = spot->p.x + WorldMap::x;
        auto spot_y = spot->p.y + WorldMap::y;

        auto dx = mouse_x - spot_x;
        auto dy = mouse_y - spot_y;

        if (std::abs(dx) <= 10 && std::abs(dy) <= 10)
        {
            return &it;
        }
    }
    return nullptr;
}

WorldMap::MapLink *WorldMap::mousein_maplink()
{
    for (auto &it : WorldMap::links)
    {
        SDL_FPoint point = {Window::mouse_x, Window::mouse_y};
        SDL_FRect rect;
        auto sprw = it.spr.sprw;
        rect.x = WorldMap::x - sprw->origin.x;
        rect.y = WorldMap::y - sprw->origin.y;
        rect.w = sprw->texture->w;
        rect.h = sprw->texture->h;
        if (SDL_PointInRectFloat(&point, &rect))
        {
            return &it;
        }
    }
    return nullptr;
}

void WorldMap::load_backgrnd()
{
    if (WorldMap::backgrnd)
    {
        SDL_DestroyTexture(WorldMap::backgrnd);
    }
    auto width = WorldMap::baseimg.spr.sprw->texture->w + 10;
    auto height = WorldMap::baseimg.spr.sprw->texture->h + 8;
    auto texture = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB4444,
                                               width,
                                               height);
    auto node = Wz::UI->get_root()->find_from_path(u"UIWindow.img/WorldMap");
    auto title = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"title")));
    node = node->find_from_path(u"Border");
    auto lt = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"0")));
    auto t = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"1")));
    auto rt = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"2")));
    auto lm = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"3")));
    auto rm = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"4")));
    auto lb = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"5")));
    auto b = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"6")));
    auto rb = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"7")));
    SDL_SetRenderTarget(Window::renderer, texture);
    SDL_FRect pos_rect;
    pos_rect.x = 0;
    pos_rect.y = 0;
    pos_rect.w = lt->w;
    pos_rect.h = lt->h;
    SDL_RenderTexture(Window::renderer, lt, nullptr, &pos_rect);
    pos_rect.x = 7;
    pos_rect.w = width - 7;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
    pos_rect.x = width - 7;
    pos_rect.w = rt->w;
    pos_rect.h = rt->h;
    SDL_RenderTexture(Window::renderer, rt, nullptr, &pos_rect);
    pos_rect.x = 0;
    pos_rect.y = 32;
    pos_rect.w = lm->w;
    pos_rect.h = height - b->h;
    SDL_RenderTexture(Window::renderer, lm, nullptr, &pos_rect);
    pos_rect.x = width - 7;
    pos_rect.y = 32;
    pos_rect.w = rm->w;
    pos_rect.h = height - b->h;
    SDL_RenderTexture(Window::renderer, rm, nullptr, &pos_rect);
    pos_rect.x = 0;
    pos_rect.y = height - b->h;
    pos_rect.w = lb->w;
    pos_rect.h = lb->h;
    SDL_RenderTexture(Window::renderer, lb, nullptr, &pos_rect);
    pos_rect.x = 7;
    pos_rect.w = width - 7;
    pos_rect.h = b->h;
    SDL_RenderTexture(Window::renderer, b, nullptr, &pos_rect);
    pos_rect.x = width - 7;
    pos_rect.w = rb->w;
    pos_rect.h = rb->h;
    SDL_RenderTexture(Window::renderer, rb, nullptr, &pos_rect);
    pos_rect.x = 6;
    pos_rect.y = 5;
    pos_rect.w = title->w;
    pos_rect.h = title->h;
    SDL_RenderTexture(Window::renderer, title, nullptr, &pos_rect);
    SDL_SetRenderTarget(Window::renderer, nullptr);
    WorldMap::backgrnd = texture;
}

void WorldMap::BtClose_func()
{
    WorldMap::hide();
}
