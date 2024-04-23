#include "sdlms/animatedsprite.hpp"
#include <ranges>
#include <algorithm>

AnimatedSprite::AnimatedSprite(std::vector<Sprite> sprite,
                               bool zigzag) : _sprite(sprite),
                                              _frameIndex(0),
                                              _frameTime(0),
                                              _frameSize(sprite.size()),
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
        _frameTime = 0;
    }

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

    return end;
}

void AnimatedSprite::draw()
{
    _sprite[_frameIndex].draw();
}

void AnimatedSprite::draw_static()
{
    _sprite[_frameIndex].draw_static();
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
AnimatedSprite AnimatedSprite::load_animated_sprite(wz::Node *node, int x, int y, int flip)
{
    std::vector<Sprite> v_sprite;

    // 从第0帧顺序读
    for (int i = 0; i < node->children_count(); i++)
    {
        auto it = node->get_child(std::to_string(i));
        if (it == nullptr)
        {
            // 如果发现没读取到,说明已经读完,则退出读取
            break;
        }
        wz::Property<wz::WzCanvas> *canvas;
        if (it->type == wz::Type::UOL)
        {
            auto uol = dynamic_cast<wz::Property<wz::WzUOL> *>(it);
            canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(uol->get_uol());
        }
        else if (it->type == wz::Type::Canvas)
        {
            canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(it);
        }
        else
        {
            continue;
        }

        Sprite sprite = Sprite::load_sprite(canvas, x, y, flip);

        v_sprite.push_back(sprite);
    }
    return AnimatedSprite(v_sprite);
}