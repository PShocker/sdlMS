#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

void Map::draw()
{
    for (auto it : _tile)
    {
        SDL_Rect rect{it.rect->x, it.rect->y, it.rect->w, it.rect->h};
        rect.x -= Camera::current()->viewport.x;
        Graphics::current()->blitSurface(it.texture, NULL, &rect);
    }
    Graphics::current()->flip();
    SDL_Delay(10);
    Graphics::current()->clear();
}