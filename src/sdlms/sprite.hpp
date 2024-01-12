#ifndef HEADER_SDLMS_SPRITE
#define HEADER_SDLMS_SPRITE

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"

class Sprite
{
public:
    Sprite(SDL_Texture *texture, SDL_Rect *rect, int format);
    void draw();

public:
    SDL_Texture *_texture;
    SDL_Rect *_rect;
    int _format;

protected:
    Camera *_camera;
    Graphics *_graphics;
};
#endif