#include "sdlms/animatedsprite.hpp"
#include <ranges>
#include <algorithm>

AnimatedSprite::AnimatedSprite(std::vector<Sprite> sprite,
                               int frameSize, bool zigzag) : _sprite(sprite),
                                                             _frameIndex(0),
                                                             _frameTime(0),
                                                             _frameSize(frameSize),
                                                             _zigzag(zigzag)
{
}

bool AnimatedSprite::update(int elapsedTime)
{
    bool end = false;
    _frameTime += elapsedTime;
    // 判断时间是否超过sprite时间
    if (_frameTime >= _sprite[_frameIndex]._delay)
    {
        // 判断是否是最后1帧
        if (_frameIndex == _frameSize - 1)
        {
            // 如果zigzag存在,说明动画是1-2-3-2-1形式的倒放
            if (_zigzag)
            {
                // 反转vector,形成倒放效果
                std::ranges::reverse(_sprite);
                // 切换到倒数第二帧
                _frameIndex = 1;
            }
            else
            {
                // 如果zigzag不存在,直接从头开始
                _frameIndex = 0;
            }
            end = true;
            // 表示一个循环结束,该值会EventSprite会使用
        }
        else
        {
            _frameIndex += 1;
        }
        // 切换下一帧
        _frameTime = 0;
    }
    else
    {
        // 透明度处理
        auto a0 = _sprite[_frameIndex]._a0;
        auto a1 = _sprite[_frameIndex]._a1;
        if (a0 != a1)
        {
            auto alpha = 255;
            if (a0 <= a1)
            {
                alpha = (float)a0 + (float)(a1 - a0) / (float)_sprite[_frameIndex]._delay * (float)_frameTime;
            }
            else
            {
                alpha = (float)a0 - (float)(a0 - a1) / (float)_sprite[_frameIndex]._delay * (float)_frameTime;
            }
            SDL_SetTextureAlphaMod(_sprite[_frameIndex]._texture, alpha);
        }
    }
    return end;
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