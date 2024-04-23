#include "sdlms/button.hpp"

Button::Button(EventSprite eventsprite, std::u16string event_key) : EventSprite(eventsprite), _event_key(event_key)
{
    _input = Input::current();
    _input->event(event_key,std::bind(&Button::event, this, std::placeholders::_1));
}

Button::~Button()
{
    _input->remove_event(_event_key);
}

void Button::event(SDL_Event &event)
{
    if (_input->isMouseHeld(SDL_BUTTON_LEFT))
    {
        int x = event.motion.x;
        int y = event.motion.y;
        SDL_FPoint p{(float)x, (float)y};
        auto rf = _eventsprite.at(_event).rect();
        if (SDL_PointInFRect(&p, &rf))
        {
            _event = Event::PRESSED;
        }
    }
    else if (_input->wasMouseReleased(SDL_BUTTON_LEFT))
    {
        int x = event.motion.x;
        int y = event.motion.y;
        SDL_FPoint p{(float)x, (float)y};
        auto rf = _eventsprite.at(_event).rect();
        if (SDL_PointInFRect(&p, &rf))
        {
            _event = Event::MOUSEOVER;
        }
        else
        {
            _event = Event::NORMAL;
        }
    }
    else if (event.type == SDL_MOUSEMOTION)
    {
        int x = event.motion.x;
        int y = event.motion.y;
        SDL_FPoint p{(float)x, (float)y};
        auto rf = _eventsprite.at(_event).rect();
        if (SDL_PointInFRect(&p, &rf))
        {
            _event = Event::MOUSEOVER;
        }
        else
        {
            _event = Event::NORMAL;
        }
    }
}
void Button::draw()
{
    _eventsprite.at(_event).draw();
}
void Button::draw_static()
{
    _eventsprite.at(_event).draw_static();
}
SDL_FRect Button::rect()
{
    return _eventsprite.at(_event).rect();
}