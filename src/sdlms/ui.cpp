#include "sdlms/ui.hpp"

UI::UI(EventSprite eventsprite) : EventSprite(eventsprite)
{
    _input = Input::current();
    _input->event(std::bind(&UI::event, this, std::placeholders::_1));
}

void UI::event(SDL_Event &event)
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
void UI::draw()
{
    _eventsprite.at(_event).draw();
}
void UI::_draw()
{
    _eventsprite.at(_event)._draw();
}
SDL_FRect UI::rect()
{
    return _eventsprite.at(_event).rect();
}