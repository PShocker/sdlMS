#ifndef HEADER_SDLMS_BACKGRD
#define HEADER_SDLMS_BACKGRD

#include <SDL2/SDL.h>
#include <vector>
#include <string>

class BackGrd
{
public:
    BackGrd(SDL_Texture *texture, SDL_Rect *rect, int format, bool front);

public:
    std::vector<SDL_Texture *> _texture;
    std::vector<SDL_Rect *> _rect;
    std::vector<int> _delay;
    std::vector<int> _format;
    int _type;
    bool front;
    int _frameIndex;
    std::u16string _url;
};
#endif