#include "sdlms/character.hpp"

Character::Character()
{
}

void Character::draw()
{
    for (auto it : _v)
    {
        if (_direct == false)
        {
            it._flip = 1;
            it._rect.x = -it._rect.x - it._rect.w;
        }
        it._rect.x += _pos.a;
        it._rect.y += _pos.b;
        it.draw();
    }
}

void Character::update(int elapsedTime)
{
    if (_animate)
    {
        auto v = _s[_status]; // tuple{vector<sprite>,delay}
        _frameTime += elapsedTime;
        if (_frameTime >= std::get<1>(v.at(_frameIndex)))
        {
            if (_frameIndex == v.size() - 1)
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
        _v = std::get<0>(v.at(_frameIndex));
    }
}

void Character::switch_type(Type type)
{
    if (type != _status)
    {
        _frameIndex = 0;
        _frameTime = 0;
        _status = type;
        _v = std::get<0>(_s[_status].at(_frameIndex));
    }
}

void Character::switch_animate()
{
    _animate = !_animate;
}
