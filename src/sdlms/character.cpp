#include "sdlms/character.hpp"

Character::Character()
{
    _input = Input::current();
    _input->event(std::bind(&event, this, std::placeholders::_1));
}

void Character::event(SDL_Event &event)
{
    if (_input->isMouseHeld(SDL_BUTTON_LEFT))
    {
        if (_status != Status::STAND)
        {
            _status = Status::STAND;
        }
    }
    else if (_input->wasMouseReleased(SDL_BUTTON_LEFT))
    {
        if (_status != Status::WALK)
        {
            _status = Status::WALK;
        }
    }
    if (_input->isKeyHeld(SDL_SCANCODE_LEFT) == true)
    {
        pos.a -= 10;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_RIGHT) == true)
    {
        pos.a += 10;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_UP) == true)
    {
        pos.b -= 10;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_DOWN) == true)
    {
        pos.b += 10;
    }
}

void Character::draw()
{
    for (auto it : _v)
    {
        it._rect.x+=pos.a;
        it._rect.y+=pos.b;
        it.draw();
    }
}