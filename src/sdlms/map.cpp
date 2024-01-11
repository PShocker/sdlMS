#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

void Map::draw()
{
    Graphics::current()->clear();
    auto camera = Camera::current();
    for (size_t i = 0; i < 8; i++)
    {
        for (auto it : _obj[i])
        {
            SDL_Rect rect{it._rect[it._frameIndex]->x, it._rect[it._frameIndex]->y, it._rect[it._frameIndex]->w, it._rect[it._frameIndex]->h};
            rect.x -= camera->viewport.x;
            Graphics::current()->blitSurface(it._texture[it._frameIndex], NULL, &rect);
        }
        for (auto it : _tile[i])
        {
            SDL_Rect rect{it._rect->x, it._rect->y, it._rect->w, it._rect->h};
            rect.x -= camera->viewport.x;
            Graphics::current()->blitSurface(it._texture, NULL, &rect);
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
            //更新帧状态
            it.update(elapsedTime);
        }
    }
    

}