#include "sdlms/animatedsprite.hpp"

AnimatedSprite::AnimatedSprite(std::vector<Sprite> sprite,
                               std::vector<int> delay,
                               int frameSize, std::vector<std::tuple<int, int>> a) : _sprite(sprite),
                                                                                     _delay(delay),
                                                                                     _frameIndex(0),
                                                                                     _frameTime(0),
                                                                                     _frameSize(frameSize),
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
    _sprite[_frameIndex].draw();
}

void AnimatedSprite::_draw()
{
    _sprite[_frameIndex]._draw();
}

SDL_FRect AnimatedSprite::rect()
{
    return SDL_FRect{(float)_sprite[_frameIndex]._rect.x, (float)_sprite[_frameIndex]._rect.y, (float)_sprite[_frameIndex]._rect.w, (float)_sprite[_frameIndex]._rect.h};
}

Sprite AnimatedSprite::sprite()
{
    return _sprite[_frameIndex];
}

Sprite AnimatedSprite::sprite(int i)
{
    return _sprite[i];
}