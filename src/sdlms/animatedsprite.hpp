#ifndef HEADER_SDLMS_ANIMATESPRITE
#define HEADER_SDLMS_ANIMATESPRITE

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "sdlms/graphics.hpp"
#include "sdlms/sprite.hpp"

class AnimatedSprite
{
public:
    AnimatedSprite(std::vector<Sprite> sprite, bool zigzag = false);

    bool update(int elapsedTime);
    void draw();
    void _draw();
    SDL_FRect rect();
    Sprite sprite();
    Sprite sprite(int index);

public:
    std::vector<Sprite> _sprite;
    int _frameIndex;
    int _frameTime;
    int _frameSize;
    bool _zigzag;
};
#endif