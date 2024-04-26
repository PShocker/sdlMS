#pragma once

#include "Components/Component.h"

class Transform : public Component
{
private:
	SDL_FPoint position;
	float rotation;
	int flip;
	unsigned z; // 根据z值渲染

public:
	Transform();
	Transform(float x, float y);
	~Transform();

	SDL_FPoint get_position() const;
	float get_rotation() const;
	int get_flip() const;

	void set_position(const SDL_FPoint &value);
	void set_rotation(const float &value);
	void set_flip(const int value);
};
