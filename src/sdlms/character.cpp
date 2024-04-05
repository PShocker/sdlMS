#include "sdlms/character.hpp"

Character::Character()
{
    _input = Input::current();
    _input->event(std::bind(&event, this, std::placeholders::_1));

    _renderer = Graphics::current()->getRenderer();
}

void Character::event(SDL_Event &event)
{
    auto status = _status;

    _status = Type::STAND1;

    _hforce = 0;
    _vforce = 0;

    if (_input->isKeyHeld(SDL_SCANCODE_LEFT) == true)
    {
        if (_ground == true)
        {
            _hforce -= 1400;
            _status = Type::WALK1;
        }
        else
        {
            _hforce -= 10;
        }
        _direct = true;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_RIGHT) == true)
    {
        if (_ground == true)
        {
            _hforce += 1400;
            _status = Type::WALK1;
        }
        else
        {
            _hforce += 10;
        }

        _direct = false;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_UP) == true)
    {
        _status = Type::WALK1;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_DOWN) == true)
    {
        _status = Type::ALERT;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_LALT) == true)
    {
        if (_ground == true)
        {
            _vspeed = -555;
        }
        _status = Type::JUMP;
    }
    if (_ground != true)
    {
        _status = Type::JUMP;
    }
    if (status != _status)
    {
        _frameIndex = 0;
        _frameTime = 0;
    }
    // 重力
    _vforce += 2000;

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
        it._rect.x += _pos.a;
        it._rect.y += _pos.b;
        it.draw();
    }
}

void Character::update(int elapsedTime)
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