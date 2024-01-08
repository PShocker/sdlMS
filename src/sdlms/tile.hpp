#include <SDL2/SDL.h>

class Tile
{
public:
    Tile(SDL_Texture *texture, SDL_Rect *rect, int format, int layer, int z);
    SDL_Texture* get_texture();
    SDL_Rect* get_rect();

private:
    SDL_Texture *texture;
    SDL_Rect *rect;
    int format;
    int layer;
    int z;
};