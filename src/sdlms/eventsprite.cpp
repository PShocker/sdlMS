#include "sdlms/eventsprite.hpp"
#include "eventsprite.hpp"

EventSprite::EventSprite(std::unordered_map<uint8_t, DynamicSprite> eventsprite) : _eventsprite(eventsprite)
{
}
EventSprite::~EventSprite()
{
}