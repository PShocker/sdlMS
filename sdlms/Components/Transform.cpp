#include "Transform.h"

Transform::Transform() : position{SDL_FPoint(0, 0)}, rotation{0} {}
Transform::Transform(float x, float y) : position{SDL_FPoint(x, y)} {}
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
