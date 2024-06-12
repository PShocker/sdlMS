#pragma once

#include "Components/Component.h"

class Transform : public Component
{
private:
	SDL_FPoint position = {0, 0};
	float rotation = 0;
	int flip = 0;
	bool camera = false;

public:
	Transform();
	Transform(SDL_FPoint p, int flip = 0, bool camera = false);
	Transform(float x, float y, int flip = 0, bool camera = false);
	~Transform();

	SDL_FPoint get_position() const;
	float get_rotation() const;
	int get_flip() const;
	auto get_camera() { return camera; };

	void set_position(const SDL_FPoint &value);
	void set_rotation(const float &value);
	void set_flip(const int value);
	void set_x(const float value) { position.x = value; };
	void set_y(const float value) { position.y = value; };
};
