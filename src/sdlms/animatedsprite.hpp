#ifndef HEADER_SDLMS_ANIMATESPRITE
#define HEADER_SDLMS_ANIMATESPRITE

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"

class AnimatedSprite
{
public:
    AnimatedSprite(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect,
                   std::vector<int> delay,
                   std::vector<int> format, int frameSize,
                   int flip);

    void update(int elapsedTime);
    void draw();

public:
    std::vector<SDL_Texture *> _texture;
    std::vector<SDL_Rect *> _rect;
    std::vector<int> _delay;
    std::vector<int> _format;
    int _frameIndex;
    int _frameTime;
    int _frameSize;
    int _flip;

protected:
    Camera *_camera;
    Graphics *_graphics;
};
#endif