#pragma once

#include "Components/Component.h"
#include "Transform.h"

class RelativeTransform : public Component
{
private:
	SDL_FPoint position;
	float rotation;
	int flip;
	Transform *tr = nullptr;

public:
	RelativeTransform(Transform *tr);
	RelativeTransform(Transform *tr, SDL_FPoint p, int flip = 0);
	RelativeTransform(Transform *tr, float x, float y, int flip = 0);
	~RelativeTransform();

	SDL_FPoint get_position() const;
	float get_rotation() const;
	int get_flip() const;
	auto get_tr() { return tr; };

	void set_position(const SDL_FPoint &value);
	void set_rotation(const float &value);
	void set_flip(const int value);
	void set_x(const float value) { position.x = value; };
	void set_y(const float value) { position.y = value; };
};
