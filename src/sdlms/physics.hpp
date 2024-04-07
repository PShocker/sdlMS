#ifndef HEADER_SDLMS_PHYSICS
#define HEADER_SDLMS_PHYSICS

#include <map>
#include <optional>
#include <SDL2/SDL.h>

#include "sdlms/character.hpp"
#include "sdlms/foothold.hpp"
#include "sdlms/map.hpp"
#include "sdlms/input.hpp"
#include "template/point.hpp"

// 实现人物或怪物的物理效果
class Physics
{

public:
    Physics();
    void update(int elapsedTime);

private:
    // 判断两条线段是否相交，并求出交点
    template <typename T>
    std::optional<Point<T>> segmentsIntersection(std::pair<const Point<T> &, const Point<T> &> a,
                                                 std::pair<const Point<T> &, const Point<T> &> b);

private:
    FootHold _fh;
    LadderRope _lp;
    enum PHYSIC_STATUS : uint8_t
    {
        GROUND,
        CLIMB,
    };
    std::bitset<8> _physic_status;

private:
    Input *_input;
    Character *_character;
    Map *_map;
};

#endif
