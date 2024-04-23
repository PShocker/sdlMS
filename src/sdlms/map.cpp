#include <ranges>

#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"
#include "util/wz_util.hpp"
#include "util/map_util.hpp"

Map::Map()
{
}

void Map::draw()
{
    // 绘制背景
    BackGrd::drawbackgrounds();

    for (size_t i = 0; i < 8; i++)
    {
        Obj::draws(i);
        Tile::draws(i);
        Npc::draws(i);
        Character::current()->draw(i);
    }
    // 绘制前景
    BackGrd::drawforegrounds();

    // 绘制传送门
    Portal::draws();

    // 绘制平台
    FootHold::draws();
    // 绘制梯子
    // LadderRope::draws(_ladderRope);
}

void Map::update(int elapsedTime)
{
    BackGrd::updates(elapsedTime);
    for (size_t i = 0; i < 8; i++)
    {
        Obj::updates(i, elapsedTime);
        Npc::updates(i, elapsedTime);
    }
    Portal::updates(elapsedTime);
}

void Map::init(int mapId)
{
    Obj::load_obj(mapId);
    Tile::load_tile(mapId);
    BackGrd::load_backgrd(mapId);
    Npc::load_npc(mapId);
    Portal::load_portal(mapId);
    FootHold::load_foothold(mapId);
    LadderRope::load_ladderRope(mapId);
    _border = util::MapUtil::current()->load_border(mapId);
}
