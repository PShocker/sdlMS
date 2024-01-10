#ifndef HEADER_SDLMS_OBJ
#define HEADER_SDLMS_OBJ

#include <SDL2/SDL.h>
#include <vector>
#include <string>

class Obj
{
public:
    Obj(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect, std::vector<int> delay,
        std::vector<int> format, int layer, int z,std::u16string url);

public:
    std::vector<SDL_Texture *> _texture;
    std::vector<SDL_Rect *> _rect;
    std::vector<int> _delay;
    std::vector<int> _format;
    int _layer;
    int _z;
    int _frameIndex;
    std::u16string _url;
};
#endif