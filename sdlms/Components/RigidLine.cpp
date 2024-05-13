#include "RigidLine.h"
#include <algorithm>

RigidLine::RigidLine(SDL_FPoint m, SDL_FPoint n)
{
    line = new Line(m, n);
}

RigidLine::~RigidLine()
{
    delete line;
}
