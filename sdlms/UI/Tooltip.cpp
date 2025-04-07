#include "ToolTip.h"

void ToolTip::run()
{
    tooltips.clear();
}

void ToolTip::push(SDL_FRect &r, SDL_Texture *texture)
{
    tooltips.push_back({r, texture});
}
