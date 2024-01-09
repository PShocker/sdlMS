#ifndef HEADER_SDLMS_TILE
#define HEADER_SDLMS_TILE

#include <SDL2/SDL.h>

class Tile
{
public:
    Tile(SDL_Texture *texture, SDL_FRect *rect, int format, int layer, int z);

public:
    SDL_Texture *texture;
    SDL_FRect *rect;
    int format;
    int layer;
    int z;
};
#endif