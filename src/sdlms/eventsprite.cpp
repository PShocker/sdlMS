#include "sdlms/eventsprite.hpp"
#include "eventsprite.hpp"

EventSprite::EventSprite(std::unordered_map<uint8_t, DynamicSprite> eventsprite) : _eventsprite(eventsprite)
{
}
EventSprite::~EventSprite()
{
}

void EventSprite::draw(uint8_t i)
{
    _eventsprite.at(i).draw();
}

void EventSprite::draw_static(uint8_t i)
{
    _eventsprite.at(i).draw_static();
}

bool EventSprite::update(uint8_t i, int elapsedTime)
{
    return _eventsprite.at(i).update(elapsedTime);
}

SDL_FRect EventSprite::rect(uint8_t i)
{
    return _eventsprite.at(i).rect();
}

EventSprite EventSprite::load_event_sprite(std::map<std::u16string, uint8_t> event_map, wz::Node *node, int x, int y)
{
    std::unordered_map<uint8_t, DynamicSprite> eventsprite;

    auto func = [x, y, &eventsprite](std::u16string str, uint8_t event, wz::Node *node) -> void
    {
        node = node->find_from_path(str);
        if (node != nullptr)
        {
            auto animatedsprite = AnimatedSprite::load_animated_sprite(node, x, y);
            auto dynamicsprite = DynamicSprite(animatedsprite);
            eventsprite.emplace(event, dynamicsprite);
        }
    };

    for (auto &it : event_map)
    {
        func(it.first, it.second, node);
    }

    return EventSprite(eventsprite);
}
