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
    auto worldmap = &World::registry->emplace<WorldMap>(ent);
    World::registry->emplace<BaseImg>(ent);

    auto node = Wz::Map->get_root()->find_from_path(u"MapHelper.img/worldMap");
    AnimatedSpriteWarp *curPos = AnimatedSpriteWarp::load(node->find_from_path(u"curPos"));
    std::optional<int> curPos_x;
    std::optional<int> curPos_y;

    // 默认是WorldMap010
    node = Wz::Map->get_root()->find_from_path(u"WorldMap/WorldMap010.img");
    auto BaseImg = SpriteWarp::load(node->find_from_path(u"BaseImg/0"));

    World::registry->emplace<Sprite>(ent, BaseImg);
    worldmap->position = SDL_FPoint{WorldMap::x, WorldMap::y};

    auto mapList = node->find_from_path(u"MapList");
    for (auto &[key, val] : mapList->get_children())
    {
        ent = World::registry->create();
        auto worldmap = &World::registry->emplace<WorldMap>(ent);
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
                curPos_x = spot_x;
                curPos_y = spot_y;
            }
            mapNo.push_back(no);
        }

        World::registry->emplace<Spot>(ent, spot_x, spot_y, type, mapNo);
        worldmap->position = SDL_FPoint{(float)spot_x + WorldMap::x, (float)spot_y + WorldMap::y};
        node = Wz::Map->get_root()->find_from_path(u"MapHelper.img/worldMap");
        switch (type)
        {
        case 0:
        {
            World::registry->emplace<Sprite>(ent, SpriteWarp::load(node->find_from_path(u"mapImage/0")));
        }
        break;
        case 1:
        {
            World::registry->emplace<Sprite>(ent, SpriteWarp::load(node->find_from_path(u"mapImage/1")));
        }
        break;
        case 2:
        {
            World::registry->emplace<Sprite>(ent, SpriteWarp::load(node->find_from_path(u"mapImage/2")));
        }
        break;
        case 3:
        {
            World::registry->emplace<Sprite>(ent, SpriteWarp::load(node->find_from_path(u"mapImage/3")));
        }
        break;
        default:
            break;
        }
    }
    if (curPos_x.has_value() && curPos_y.has_value())
    {
        // 玩家所在的位置动画
        ent = World::registry->create();
        World::registry->emplace<CurPos>(ent);

        auto worldmap = &World::registry->emplace<WorldMap>(ent);
        World::registry->emplace<Animated>(ent);
        World::registry->emplace<AnimatedSprite>(ent, curPos);
        worldmap->position = SDL_FPoint{(float)curPos_x.value() + WorldMap::x, (float)curPos_y.value() + WorldMap::y};
    }

    WorldMap::open = true;
}

void WorldMap::hide()
{
    // 销毁所有具有 WorldMap 组件的实体
    for (auto ent : World::registry->view<WorldMap>())
    {
        World::destory.push_back(ent);
    }
    WorldMap::open = false;
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

bool WorldMap::over()
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
            return true;
        }
    }
    return false;
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
