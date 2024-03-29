#ifndef HEADER_SDLMS_PHYSICS
#define HEADER_SDLMS_PHYSICS

#include <map>

#include "sdlms/foothold.hpp"


// 实现人物或怪物的物理效果
class Physics
{
    Physics();

    static std::unordered_map<std::pair<int, int>, FootHold> fh;
};

#endif