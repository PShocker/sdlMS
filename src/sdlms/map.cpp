#include "sdlms/map.hpp"
#include "sdlms/camera.hpp"

void Map::draw()
{
    Graphics::current()->clear();
    auto camera = Camera::current();
    for (auto it : _obj)
    {
        SDL_Rect rect{it._rect[0]->x, it._rect[0]->y, it._rect[0]->w, it._rect[0]->h};
        rect.x -= camera->viewport.x;
        Graphics::current()->blitSurface(it._texture[0], NULL, &rect);
    }

    for (auto it : _tile)
    {
        SDL_Rect rect{it._rect->x, it._rect->y, it._rect->w, it._rect->h};
        rect.x -= camera->viewport.x;
        Graphics::current()->blitSurface(it._texture, NULL, &rect);
    }
    Graphics::current()->flip();
}