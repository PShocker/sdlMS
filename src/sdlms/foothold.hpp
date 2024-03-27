#ifndef HEADER_SDLMS_FOOTHOLD
#define HEADER_SDLMS_FOOTHOLD

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"

#include "template/point.hpp"

class FootHold
{
public:
    FootHold(Point<int32_t> a, Point<int32_t> b, int page, int zmass, int id, int prev, int next);

    void draw();

public:
    Point<int32_t> _a;
    Point<int32_t> _b;
    int _page;
    int _zmass;
    int _id;
    int _prev;
    int _next;

    int _len;
    int _uvx; // sinx
    int _uvy; // siny
              /*
              fh.len =  sqrt((x2-x1)^2 + (y2-y1)^2)
              fh.uvx = (fh.x2 - fh.x1) / fh.len
              fh.uvy = (fh.y2 - fh.y1) / fh.len
          
              fh.uvx > 0: 地面
              fh.uvx == 0: 墙。fh.uvy > 0:  从右往左会被阻挡的墙；fh.uvy < 0:  从左往右会被阻挡的墙；
              fh.uvx < 0: 天花板
          
              当角色在地面上走的时候，只能走到fh.next和fh.last
              当角色在空中的时候，能跟所有uvx > 0的平台碰撞，但是只能跟（ ”zmass= 0“ 或 ”zmass = 起跳平台.zmass“） 的uvx <= 0的平台碰撞
              */
private:
    Camera *_camera;
    SDL_Renderer *_renderer;
    
};
#endif