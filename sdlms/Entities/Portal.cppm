module;

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

export module entities:portal;

export
{
    void load_portal(wz::Node * node, int id);
    void fix_portal();
    SDL_FPoint recent_portal(float x,float y);
};