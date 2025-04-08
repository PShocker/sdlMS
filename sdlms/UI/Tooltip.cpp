#include "ToolTip.h"
#include "Cursor.h"
#include "Resources/Wz.h"
#include "Core/Map.h"

void ToolTip::run()
{
    tooltips.clear();
}

void ToolTip::show()
{
    load_maptooltip();
}

void ToolTip::over()
{
    SDL_FPoint point = {Cursor::x, Cursor::y};
    for (auto &[r, texture] : map_tooltips)
    {
        SDL_FRect rect;
        rect.x = r.x - Camera::x;
        rect.y = r.y - Camera::y;
        rect.w = r.w;
        rect.h = r.h;
        if (SDL_PointInRectFloat(&point, &rect))
        {
            rect.x = Cursor::x;
            rect.y = Cursor::y;
            tooltips.push_back({rect, texture});
        }
    }
}

void ToolTip::load_maptooltip()
{
    for (auto &[r, texture] : map_tooltips)
    {
        SDL_DestroyTexture(texture);
    }
    map_tooltips.clear();

    auto node = Map::load_map_node(Map::id);
    if (node->get_child(u"ToolTip"))
    {
        node = node->get_child(u"ToolTip");
        for (auto &[key, val] : node->get_children())
        {
            auto x1 = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"x1"))->get();
            auto y1 = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"y1"))->get();
            auto x2 = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"x2"))->get();
            auto y2 = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"y2"))->get();

            auto n = Wz::String->get_root()->find_from_path("ToolTipHelp.img/Mapobject/" + std::to_string(Map::id));
            if (n->get_child(key))
            {
                n = n->get_child(key);
                auto title = dynamic_cast<wz::Property<wz::wzstring> *>(n->get_child(u"Title"))->get();
                SDL_FRect r{(float)x1, (float)y1, (float)x2 - x1, (float)y2 - y1};
                auto texture = FreeType::load(title, SDL_Color{230, 202, 72, 255}, 0, 12);
                map_tooltips.push_back({r, texture});
            }
        }
    }
}

void ToolTip::push(SDL_FRect &r, SDL_Texture *texture)
{
    tooltips.push_back({r, texture});
}
