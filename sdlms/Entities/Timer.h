#pragma once
#include "Entity.h"

class Timer : public Entity
{
public:
    Timer(){};
    Timer(SDL_TimerID timer_id);
    void set_timer_id(SDL_TimerID value) { timer_id = value; };
    constexpr auto get_timer_id() { return timer_id; }

private:
    SDL_TimerID timer_id;
};