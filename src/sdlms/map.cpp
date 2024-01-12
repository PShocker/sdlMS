#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

void Map::draw()
{
    Graphics::current()->clear();
    for (auto it : _backgrd)
    {
        it.draw();
    }
    for (size_t i = 0; i < 8; i++)
    {
        for (auto it : _obj[i])
        {
            it.draw();
        }
        for (auto it : _tile[i])
        {
            it.draw();
        }
    }
    Graphics::current()->flip();
}

void Map::update(int elapsedTime)
{
    for (size_t i = 0; i < 8; i++)
    {
        for (auto &it : _obj[i])
        {
            // 更新帧状态
            it.update(elapsedTime);
        }
    }
    for (auto &it : _backgrd)
    {
        it.update(elapsedTime);
    }
}