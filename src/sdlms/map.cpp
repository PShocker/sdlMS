#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"
#include "map.hpp"

Map::Map()
{
}

void Map::draw()
{
    Graphics::current()->clear();
    for (auto &it : _backgrd)
    {
        it.draw();
    }
    for (size_t i = 0; i < 8; i++)
    {
        for (auto &it : _tile_obj[i])
        {
            if (std::holds_alternative<Obj>(it))
            {
                std::get<Obj>(it).draw();
            }
            else if (std::holds_alternative<Tile>(it))
            {
                std::get<Tile>(it).draw();
            }
        }
    }
    for (auto &it : _portal)
    {
        it.draw();
    }
    Graphics::current()->flip();
}

void Map::update(int elapsedTime)
{
    for (auto &it : _backgrd)
    {
        it.update(elapsedTime);
    }
    for (size_t i = 0; i < 8; i++)
    {
        for (auto &it : _obj[i])
        {
            // 更新帧状态
            it.update(elapsedTime);
        }
    }
    for (auto &it : _portal)
    {
        it.update(elapsedTime);
    }
}

void Map::event(const SDL_Event &event)
{
    for (auto &it : _portal)
    {
        it.event(event);
    }
}