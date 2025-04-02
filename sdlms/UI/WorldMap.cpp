#include "WorldMap.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Systems/UI.h"

void WorldMap::show()
{
    WorldMap::x = Camera::w / 2;
    WorldMap::y = Camera::h / 2;

    auto node = Wz::Map->get_root()->find_from_path(u"MapHelper.img/worldMap");
    // 默认是WorldMap010
    node = Wz::Map->get_root()->find_from_path(u"WorldMap/WorldMap010.img");
    WorldMap::baseimg.spr = Sprite(node->find_from_path(u"BaseImg/0"));

    WorldMap::spots.clear();
    auto mapList = node->find_from_path(u"MapList");
    for (auto &[key, val] : mapList->get_children())
    {
        Spot spt;
        auto spot = val[0]->get_child(u"spot");
        auto type = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"type"))->get();
        auto spot_v = dynamic_cast<wz::Property<wz::WzVec2D> *>(spot)->get();
        auto spot_x = spot_v.x;
        auto spot_y = spot_v.y;
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
                WorldMap::curpos.x = spot_x;
                WorldMap::curpos.y = spot_y;
                WorldMap::curpos.aspr = AnimatedSprite(Wz::Map->get_root()->find_from_path(u"MapHelper.img/worldMap/curPos"));
            }
            mapNo.push_back(no);
        }
        spt.x = spot_x;
        spt.y = spot_y;
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
    if (WorldMap::open)
    {
        float mouse_x = Window::mouse_x;
        float mouse_y = Window::mouse_y;
        // 判断是否点击到spot
        for (auto &it : WorldMap::spots)
        {
            auto spot = &it;
            auto spot_x = spot->x + WorldMap::x;
            auto spot_y = spot->y + WorldMap::y;

            auto dx = mouse_x - spot_x;
            auto dy = mouse_y - spot_y;

            if (std::abs(dx) <= 10 && std::abs(dy) <= 10)
            {
                // switch map
                World::TransPort::id = spot->mapNo[0];
                World::TransPort::tn = u"sp";
                break;
            }
        }
        if (World::TransPort::id != 0)
        {
            WorldMap::hide();
        }
    }
}

void WorldMap::over()
{
    if (WorldMap::open)
    {
        float mouse_x = Window::mouse_x;
        float mouse_y = Window::mouse_y;
        // 判断是否点击到spot
        for (auto &it : WorldMap::spots)
        {
            auto spot = &it;
            auto spot_x = spot->x + WorldMap::x;
            auto spot_y = spot->y + WorldMap::y;

            auto dx = mouse_x - spot_x;
            auto dy = mouse_y - spot_y;

            if (std::abs(dx) <= 10 && std::abs(dy) <= 10)
            {
                Cursor::type = u"1";
            }
        }
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
