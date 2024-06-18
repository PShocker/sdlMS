#include "Transform.h"

Transform::Transform() : position({0, 0}), rotation(0), flip(0) {}
Transform::Transform(SDL_FPoint p, int flip, bool camera) : position(p), rotation(0), flip(0) {}
Transform::Transform(float x, float y, int flip, bool camera) : position({x, y}), rotation(0), flip(flip), camera(camera) {}
Transform::~Transform() {}

float Transform::get_rotation() const
{
	return rotation;
}

int Transform::get_flip() const
{
	return flip;
}

void Transform::set_position(const SDL_FPoint &value)
{
	position = value;
}
SDL_FPoint Transform::get_position() const
{
	return position;
}

void Transform::set_rotation(const float &value)
{
	rotation = value;
}

void Transform::set_flip(const int value)
{
	flip = value;
}
