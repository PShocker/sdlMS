#include "WorldMap.h"
#include "wz/Property.hpp"
#include <SDL3/SDL.h>
#include "Resources/Wz.h"
#include "Components/Components.h"
#include "Commons/Commons.h"

void WorldMap::show()
{
    WorldMap::x = Camera::w / 2;
    WorldMap::y = Camera::h / 2;

    auto ent = World::registry->create();
    World::registry->emplace<WorldMap>(ent);

    SpriteWarp *mapImage[3];
    auto node = Wz::Map->get_root()->find_from_path(u"MapHelper.img/worldMap");
    mapImage[0] = SpriteWarp::load(node->find_from_path(u"mapImage/0"));
    mapImage[1] = SpriteWarp::load(node->find_from_path(u"mapImage/1"));
    mapImage[2] = SpriteWarp::load(node->find_from_path(u"mapImage/2"));

    // AnimatedSpriteWarp *curPos = AnimatedSpriteWarp::load(node->find_from_path(u"curPos"));

    // 默认是WorldMap010
    node = Wz::Map->get_root()->find_from_path(u"WorldMap/WorldMap010.img");
    auto BaseImg = SpriteWarp::load(node->find_from_path(u"BaseImg/0"));

    auto &spr = World::registry->emplace<Sprite>(ent, BaseImg);
    World::registry->emplace<Transform>(ent, WorldMap::x, WorldMap::y, UI_Z, 0, true);

    auto mapList = node->find_from_path(u"MapList");
    for (auto &[key, val] : mapList->get_children())
    {
        auto ent = World::registry->create();
        World::registry->emplace<WorldMap>(ent);
        auto spot = val[0]->get_child(u"spot");
        auto type = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"type"))->get();
        auto spot_v = dynamic_cast<wz::Property<wz::WzVec2D> *>(spot)->get();
        auto spot_x = spot_v.x;
        auto spot_y = spot_v.y;
        auto mapNo = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"mapNo")->get_child(u"0"))->get();
        World::registry->emplace<Spot>(ent, spot_x, spot_y, type, mapNo);
        SpriteWarp *sprw = mapImage[type];
        World::registry->emplace<Sprite>(ent, sprw);
        World::registry->emplace<Transform>(ent, (float)spot_x + WorldMap::x, (float)spot_y + WorldMap::y, UI_Z + 1, 0, true);
    }
    WorldMap::open = true;
    World::zindex = true;
}

void WorldMap::hide()
{
    // 销毁所有具有 WorldMap 组件的实体
    for (auto ent : World::registry->view<WorldMap>())
    {
        World::registry->destroy(ent);
    }
    WorldMap::open = false;
    World::zindex = true;
}

void WorldMap::click()
{
    float mouse_x = Window::mouse_x;
    float mouse_y = Window::mouse_y;
    // 判断是否点击到spot
    for (auto ent : World::registry->view<Spot>())
    {
        auto spot = World::registry->try_get<Spot>(ent);
        auto spot_x = spot->x + WorldMap::x;
        auto spot_y = spot->y + WorldMap::y;

        auto dx = mouse_x - spot_x;
        auto dy = mouse_y - spot_y;

        if (std::abs(dx) <= 10 && std::abs(dy) <= 10)
        {
            // switch map
            World::TransPort::id = spot->mapNo;
            break;
        }
    }
    if (World::TransPort::id != 0)
    {
        WorldMap::hide();
    }
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
