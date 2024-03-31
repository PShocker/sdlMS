#ifndef HEADER_SDLMS_PHYSICS
#define HEADER_SDLMS_PHYSICS

#include <map>

#include "sdlms/character.hpp"
#include "sdlms/foothold.hpp"
#include "sdlms/map.hpp"

// 实现人物或怪物的物理效果
class Physics
{

public:
    Physics();
    void update();


private:
    std::map<std::pair<int, int>, FootHold *> _fh;

private:
    Character *_character;
    Map *_map;
};

#endif