#include "RelativeTransform.h"

RelativeTransform::RelativeTransform(Transform *tr) : tr(tr), position({0.0f, 0.0f}), rotation(0), flip(0) {}
RelativeTransform::RelativeTransform(Transform *tr, SDL_FPoint p, int flip) : tr(tr), position(p), rotation(0), flip(flip) {}
RelativeTransform::RelativeTransform(Transform *tr, float x, float y, int flip) : tr(tr), position({x, y}), rotation(0), flip(flip) {}
RelativeTransform::~RelativeTransform() {}

float RelativeTransform::get_rotation() const
{
    return rotation;
}

int RelativeTransform::get_flip() const
{
    return flip;
}

void RelativeTransform::set_position(const SDL_FPoint &value)
{
    position = value;
}
SDL_FPoint RelativeTransform::get_position() const
{
    return position;
}

void RelativeTransform::set_rotation(const float &value)
{
    rotation = value;
}

void RelativeTransform::set_flip(const int value)
{
    flip = value;
}