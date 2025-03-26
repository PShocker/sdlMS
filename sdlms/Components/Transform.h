#pragma once

#include <SDL3/SDL.h>

struct Transform
{
	SDL_FPoint position = {0, 0};
	float rotation = 0;
	int flip = 0;
	int z = 0;

	Transform() = default;
	Transform(SDL_FPoint p, int z = 0, int flip = 0);
	Transform(float x, float y, int z = 0, int flip = 0);
	~Transform() = default;
};
