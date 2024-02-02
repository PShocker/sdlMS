#include "sdlms/eventsprite.hpp"

EventSprite::EventSprite(std::map<Event, DynamicSprite *> eventsprite) : _eventsprite(eventsprite)
{
    _camera = Camera::current();
}

void EventSprite::event(SDL_Event &event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        int x = event.motion.x;
        int y = event.motion.y;
        SDL_FPoint p{(float)x + _camera->_viewport.x, (float)y + _camera->_viewport.y};
        // auto rf = _eventsprite[_event];
        auto rf = _eventsprite[_event]->rect();
        if (SDL_PointInFRect(&p, &rf))
        {
            _event=Event::MOUSEOVER;
            printf("EventSprite");
        }
    }
}