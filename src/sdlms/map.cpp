#include <ranges>

#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

Map::Map()
{
}

void Map::draw()
{
    // 绘制背景
    BackGrd::drawbackgrounds(_backgrd.first);

    for (size_t i = 0; i < 8; i++)
    {
        Obj::draws(_obj[i]);
        Tile::draws(_tile[i]);
        Npc::draws(_npc[i]);
        Character::current()->draw(i);
    }
    // 绘制前景
    BackGrd::drawbackgrounds(_backgrd.second);

    // 绘制传送门
    Portal::draws(_portal);

    // 绘制平台
    // FootHold::draws(_foothold);
    // 绘制梯子
    // LadderRope::draws(_ladderRope);
}

void Map::update(int elapsedTime)
{
    BackGrd::updates(_backgrd.first, elapsedTime);  // 更新背景
    BackGrd::updates(_backgrd.second, elapsedTime); // 更新前景
    for (size_t i = 0; i < 8; i++)
    {
        Obj::updates(_obj[i], elapsedTime);
        Npc::updates(_npc[i], elapsedTime);
    }
    Portal::updates(_portal, elapsedTime);
}