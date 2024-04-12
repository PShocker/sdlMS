#ifndef HEADER_SDLMS_ROPE
#define HEADER_SDLMS_ROPE

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"

class LadderRope
{
public:
    LadderRope();
    LadderRope(int id, int l, int uf, int x, int y1, int y2, int page);
    static void draws(std::unordered_map<int, LadderRope> &r);

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