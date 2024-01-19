#include "sdlms/animatedsprite.hpp"

AnimatedSprite::AnimatedSprite(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect> rect,
                               std::vector<int> delay, std::vector<int> format,
                               int frameSize, std::vector<std::tuple<int, int>> a,
                               int flip) : _texture(texture), _rect(rect),
                                           _delay(delay), _format(format),
                                           _frameIndex(0),
                                           _frameTime(0),
                                           _frameSize(frameSize),
                                           _flip(flip),
                                           _a(a)
{
    _camera = Camera::current();
    _graphics = Graphics::current();
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
            SDL_SetTextureAlphaMod(_texture[_frameIndex], alpha);
        }
    }
}

void AnimatedSprite::draw()
{
    auto fr = rect();
    // SDL_FRect rect{(float)_rect[_frameIndex].x, (float)_rect[_frameIndex].y, (float)_rect[_frameIndex].w, (float)_rect[_frameIndex].h};
    fr.x -= _camera->viewport.x;
    fr.y -= _camera->viewport.y;
    if (_flip > 0) // 翻转
    {
        _graphics->blitSurfaceEx(_texture[_frameIndex], NULL, &fr, 0, 0, SDL_FLIP_HORIZONTAL);
    }
    else
    {
        _graphics->blitSurface(_texture[_frameIndex], NULL, &fr);
    }
}

SDL_FRect AnimatedSprite::rect()
{
    return SDL_FRect{(float)_rect[_frameIndex].x, (float)_rect[_frameIndex].y, (float)_rect[_frameIndex].w, (float)_rect[_frameIndex].h};
}
