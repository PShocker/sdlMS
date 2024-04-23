#ifndef HEADER_SDLMS_FRAME
#define HEADER_SDLMS_FRAME

#include "sdlms/sprite.hpp"
#include "wz/Property.hpp"

class Frame
{
public:
    Frame(){};
    Frame(wz::Node *node);
    void draw(Point<float> position, int16_t rwidth, int16_t rheight);
    void draw_static(Point<float> position, int16_t rwidth, int16_t rheight);

public:
    Sprite center;
    Sprite east;
    Sprite northeast;
    Sprite north;
    Sprite northwest;
    Sprite west;
    Sprite southwest;
    Sprite south;
    Sprite southeast;

    int16_t xtile;
    int16_t ytile;
};
#endif
