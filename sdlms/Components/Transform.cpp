#include "Transform.h"

Transform::Transform() : position{Point<float>(0, 0)}, rotation{0} {}
Transform::~Transform() {}

float Transform::get_rotation() const
{
	return rotation;
}

void Transform::set_position(const Point<float> &value)
{
	position = value;
}
Point<float> Transform::get_position() const
{
	return position;
}

void Transform::set_rotation(const float &value)
{
	rotation = value;
}