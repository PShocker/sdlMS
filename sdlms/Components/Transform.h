#pragma once
#include <SDL3/SDL.h>

struct Transform
{
	SDL_FPoint position = {0, 0};
	float rotation = 0;
	int flip = 0;
	bool camera = false;

	Transform()=default;
	Transform(SDL_FPoint p, int flip = 0, bool camera = false);
	Transform(float x, float y, int flip = 0, bool camera = false);
	~Transform()=default;
};
