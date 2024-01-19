#ifndef HEADER_SDLMS_SPRITE
#define HEADER_SDLMS_SPRITE

#include <SDL2/SDL.h>
#include <vector>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"

class Sprite
{
public:
    Sprite(const std::vector<uint8_t> &raw_data, SDL_FRect rect, unsigned int format);
    Sprite(SDL_Texture *texture, SDL_FRect rect);

    void draw();

    SDL_FRect rect();

public:
    SDL_Texture *_texture;
    SDL_FRect _rect;
    unsigned int _format;
};
#endif