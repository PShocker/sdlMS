#include "sdlms/character.hpp"

Character::Character()
{
    _input = Input::current();
    _input->event(std::bind(&event, this, std::placeholders::_1));
}

void Character::event(SDL_Event &event)
{
    auto status = _status;

    _status = Type::STAND1;

    if (_input->isKeyHeld(SDL_SCANCODE_LEFT) == true)
    {
        pos.a -= 3;
        _status = Type::WALK1;
        _direct = true;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_RIGHT) == true)
    {
        pos.a += 3;
        _status = Type::WALK1;
        _direct = false;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_UP) == true)
    {
        pos.b -= 3;
        _status = Type::WALK1;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_DOWN) == true)
    {
        pos.b += 3;
        _status = Type::WALK1;
    }
    if (status != _status)
    {
        _frameIndex = 0;
        _frameTime = 0;
    }

    return;
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
        it._rect.x += pos.a;
        it._rect.y += pos.b;
        it.draw();
    }
}
void Character::update(int elapsedTime)
{
    auto v = _s[_status]; //tuple{vector<sprite>,delay}
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