#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"
#include "map.hpp"

Map::Map()
{
}

void Map::draw()
{
    Graphics::current()->clear();
    // 绘制背景
    for (auto &it : _backgrd)
    {
        it.draw(false);
    }
    for (size_t i = 0; i < 8; i++)
    {
        for (auto &it : _obj[i])
        {
            it.draw();
        }
        for (auto &it : _tile[i])
        {
            it.draw();
        }
    }
    // 绘制前景
    for (auto &it : _backgrd)
    {
        it.draw(true);
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