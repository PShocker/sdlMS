#include "sdlms/animatedsprite.hpp"

AnimatedSprite::AnimatedSprite(std::vector<Sprite> sprite,
                               std::vector<int> delay,
                               int frameSize, std::vector<std::tuple<int, int>> a,
                               int flip) : _sprite(sprite),
                                           _delay(delay),
                                           _frameIndex(0),
                                           _frameTime(0),
                                           _frameSize(frameSize),
                                           _flip(flip),
                                           _a(a)
{
}

void AnimatedSprite::update(int elapsedTime)
{
    if (_frameSize != 1)
    {
        _frameTime += elapsedTime;
        if (_frameTime >= _delay[_frameIndex])
        {
            if (_frameIndex == _frameSize - 1)
            {
                _frameIndex = 0;
            }
            else
            {
                _frameIndex += 1;
            }
            // 切换下一帧
            _frameTime = 0;
        }
        auto a0 = std::get<0>(_a[_frameIndex]);
        auto a1 = std::get<1>(_a[_frameIndex]);
        if (a0 != a1)
        {
            auto alpha = 255;
            if (a0 <= a1)
            {
                alpha = (float)a0 + (float)(a1 - a0) / (float)_delay[_frameIndex] * (float)_frameTime;
            }
            else
            {
                alpha = (float)a0 - (float)(a0 - a1) / (float)_delay[_frameIndex] * (float)_frameTime;
            }
            SDL_SetTextureAlphaMod(_sprite[_frameIndex]._texture, alpha);
        }
    }
}

void AnimatedSprite::draw()
{
    auto camera = Camera::current();
    auto graphics = Graphics::current();
    auto fr = rect();
    // SDL_FRect rect{(float)_rect[_frameIndex].x, (float)_rect[_frameIndex].y, (float)_rect[_frameIndex].w, (float)_rect[_frameIndex].h};
    fr.x -= camera->viewport.x;
    fr.y -= camera->viewport.y;
    if (_flip > 0) // 翻转
    {
        graphics->blitSurfaceEx(_sprite[_frameIndex]._texture, NULL, &fr, 0, 0, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        graphics->blitSurface(_sprite[_frameIndex]._texture, NULL, &fr);
    }
}

SDL_FRect AnimatedSprite::rect()
{

    return SDL_FRect{(float)_sprite[_frameIndex]._rect.x, (float)_sprite[_frameIndex]._rect.y, (float)_sprite[_frameIndex]._rect.w, (float)_sprite[_frameIndex]._rect.h};
}
