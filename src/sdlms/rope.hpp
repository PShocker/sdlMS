#ifndef HEADER_SDLMS_ROPE
#define HEADER_SDLMS_ROPE

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"

class Rope
{
public:
    Rope();
    Rope(int id, int l, int uf, int x, int y1, int y2, int page);
    void draw();

public:
    int _id;
    int _l;
    int _uf;
    int _x;
    int _y1;
    int _y2;
    int _page;

private:
    Camera *_camera;
    SDL_Renderer *_renderer;
};
#endif