#include "Transform.h"

Transform::Transform() : position{Vector2(0, 0)}, rotation{0}, scale{Vector2(1, 1)}, parent{nullptr} {}
Transform::~Transform() {}


Vector2 Transform::get_position() const {
	return position;
}

Vector2 Transform::get_position_from_parent() const {
	return parent ? parent->position + position : position;
}

float Transform::get_rotation() const {
	return rotation;
}

Vector2 Transform::get_scale() const {
	return scale;
}


void Transform::set_position(const Vector2& value) {
	position = value;
}

void Transform::set_rotation(const float& value) {
	rotation = value;
}

void Transform::set_scale(const Vector2& value) {
	scale = value;
}

Transform* Transform::get_parent() const {
	return parent;
}

void Transform::set_parent(Transform* value) {
	parent = value;
}
