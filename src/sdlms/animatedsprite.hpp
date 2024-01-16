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
    AnimatedSprite(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect> rect,
                   std::vector<int> delay,
                   std::vector<int> format, int frameSize,
                   std::vector<std::tuple<int, int>> a,
                   int flip = SDL_FLIP_NONE);

    void update(int elapsedTime);
    void draw();

public:
    std::vector<SDL_Texture *> _texture;
    std::vector<SDL_Rect> _rect;
    std::vector<int> _delay;
    std::vector<int> _format;
    int _frameIndex;
    int _frameTime;
    int _frameSize;
    int _flip;

    std::vector<std::tuple<int, int>> _a;

protected:
    Camera *_camera;
    Graphics *_graphics;
};
#endif