#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

void Map::draw()
{
    for (auto it : _tile)
    {
        SDL_FRect rect{it.rect->x, it.rect->y, it.rect->w, it.rect->h};
        rect.x -= Camera::current()->x;
        Graphics::current()->blitSurface(it.texture, NULL, &rect);
    }
    Graphics::current()->flip();
    SDL_Delay(16);
    Graphics::current()->clear();
}