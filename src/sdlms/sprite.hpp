#ifndef HEADER_SDLMS_SPRITE
#define HEADER_SDLMS_SPRITE

#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"

class Sprite
{
public:
    Sprite(SDL_Texture *texture, SDL_FRect rect, int format);
    ~Sprite();
    
    void draw();

    SDL_FRect rect();

public:
    SDL_Texture *_texture;
    // SDL_FRect *_rect;
    SDL_FRect _rect;
    int _format;

protected:
    Camera *_camera;
    Graphics *_graphics;
};
#endif