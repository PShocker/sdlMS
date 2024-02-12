#include "sdlms/character.hpp"

Character::Character()
{
    _camera = Camera::current();

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
}

void Character::load()
{
    
}

void Character::draw(){
    for (auto &it:_v)
    {
        it._draw();
    }
    
}