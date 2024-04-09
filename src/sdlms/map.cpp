#include <ranges>

#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

Map::Map()
{
}

void Map::draw()
{
    // 绘制背景
    BackGrd::drawbackgrounds(_backgrd);
    for (size_t i = 0; i < 8; i++)
    {
        Obj::draws(_obj[i]);
        Tile::draws(_tile[i]);
        Character::current()->draw(i);
    }
    // 绘制前景
    BackGrd::drawforegrounds(_backgrd);

    // 绘制传送门
    Portal::draws(_portal);

    // 绘制平台
    // FootHold::draws(_foothold);
    // 绘制梯子
    // LadderRope::draws(_ladderRope);
}

void Map::update(int elapsedTime)
{
    BackGrd::updates(_backgrd, elapsedTime);
    for (size_t i = 0; i < 8; i++)
    {
        Obj::updates(_obj[i], elapsedTime);
    }
    Portal::updates(_portal, elapsedTime);
}

void Map::event(const SDL_Event &event)
{
    for (auto &it : _portal)
    {
        it.event(event);
    }
}