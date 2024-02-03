#include "sdlms/eventsprite.hpp"

EventSprite::EventSprite(std::map<Event, DynamicSprite> eventsprite) : _eventsprite(eventsprite)
{
    _camera = Camera::current();

    _input = Input::current();
    _input->event(std::bind(&event, this, std::placeholders::_1));
}

void EventSprite::event(SDL_Event &event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        int x = event.motion.x;
        int y = event.motion.y;
        SDL_FPoint p{(float)x + _camera->_viewport.x, (float)y + _camera->_viewport.y};
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
void EventSprite::draw()
{
    _eventsprite.at(_event).draw();
}
void EventSprite::_draw()
{
    _eventsprite.at(_event)._draw();
}
SDL_FRect EventSprite::rect()
{
    return _eventsprite.at(_event).rect();
}