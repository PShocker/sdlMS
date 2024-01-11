#ifndef HEADER_SDLMS_BACKGRD
#define HEADER_SDLMS_BACKGRD

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"

class BackGrd
{
public:
    BackGrd(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect, std::vector<int> delay,
            std::vector<int> format, int type, int front,
            int frameIndex, int frameSize,
            std::u16string url);
    void draw();

public:
    std::vector<SDL_Texture *> _texture;
    std::vector<SDL_Rect *> _rect;
    std::vector<int> _delay;
    std::vector<int> _format;
    int _type;
    int _front;
    int _frameIndex;
    int _frameSize;
    int _frameTime;
    std::u16string _url;

private:
    Camera *_camera;
};
#endif