#include "sdlms/button.hpp"
#include "button.hpp"

Button::Button(EventSprite eventsprite, std::u16string event_key) : _es(eventsprite), _event_key(event_key)
{
    _input = Input::current();
    _input->event(event_key, std::bind(&Button::event, this, std::placeholders::_1));
}

Button &Button::operator=(Button &&other) noexcept
{
    _input = Input::current();
    _input->event(other._event_key, std::bind(&Button::event, this, std::placeholders::_1));

    _es = std::move(other._es);
    _event_key = std::move(other._event_key);

    return *this;
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
        auto rf = _es.rect(_event);
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
        auto rf = _es.rect(_event);
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
        auto rf = _es.rect(_event);
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
    _es.draw(_event);
}
void Button::draw_static()
{
    _es.draw_static(_event);
}
SDL_FRect Button::rect()
{
    return _es.rect(_event);
}