#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

void Map::draw()
{
    Graphics::current()->clear();
    for (auto it : _tile)
    {
        SDL_Rect rect{it.rect->x, it.rect->y, it.rect->w, it.rect->h};
        rect.x -= Camera::current()->viewport.x;
        Graphics::current()->blitSurface(it.texture, NULL, &rect);
    }
    Graphics::current()->flip();
}