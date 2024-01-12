#include "sdlms/animatedsprite.hpp"
#include "animatedsprite.hpp"

AnimatedSprite::AnimatedSprite(std::vector<SDL_Texture *> texture, std::vector<SDL_Rect *> rect,
                               std::vector<int> delay, std::vector<int> format,
                               int frameSize) : _texture(texture), _rect(rect),
                                                _delay(delay), _format(format),
                                                _frameIndex(0),
                                                _frameTime(0),
                                                _frameSize(frameSize)
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
    }
}

void AnimatedSprite::draw()
{
    SDL_Rect rect{_rect[_frameIndex]->x, _rect[_frameIndex]->y, _rect[_frameIndex]->w, _rect[_frameIndex]->h};
    rect.x -= _camera->viewport.x;
    rect.y -= _camera->viewport.y;
    _graphics->blitSurface(_texture[_frameIndex], NULL, &rect);
}
