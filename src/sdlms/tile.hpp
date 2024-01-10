#ifndef HEADER_SDLMS_TILE
#define HEADER_SDLMS_TILE

#include <SDL2/SDL.h>

class Tile
{
public:
    Tile(SDL_Texture *texture, SDL_Rect *rect, int format, int layer, int z);

public:
    SDL_Texture *_texture;
    SDL_Rect *_rect;
    int _format;
    int _layer;
    int _z;
    
};
#endif