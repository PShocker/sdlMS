#ifndef HEADER_SDLMS_ANIMATESPRITE
#define HEADER_SDLMS_ANIMATESPRITE

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "sdlms/graphics.hpp"
#include "sdlms/camera.hpp"
#include "sdlms/sprite.hpp"

class AnimatedSprite
{
public:
    AnimatedSprite(std::vector<Sprite> sprite,
                   std::vector<int> delay,
                   int frameSize,
                   std::vector<std::tuple<int, int>> a);

    void update(int elapsedTime);
    void draw();
    void _draw();
    SDL_FRect rect();

public:
    std::vector<Sprite> _sprite;
    std::vector<int> _delay;
    int _frameIndex;
    int _frameTime;
    int _frameSize;

    std::vector<std::tuple<int, int>> _a;
};
#endif