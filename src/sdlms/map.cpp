#include <ranges>

#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

Map::Map()
{
}

void Map::draw()
{
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
    // 绘制传送门
    for (auto &it : _portal)
    {
        it.draw();
    }
    // 绘制平台
    for (auto [_, it] : _foothold)
    {
        it.draw();
    }
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