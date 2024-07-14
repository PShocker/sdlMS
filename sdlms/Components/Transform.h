#pragma once
#include <SDL3/SDL.h>

struct Transform
{
	SDL_FPoint position = {0, 0};
	float rotation = 0;
	int flip = 0;
	bool camera = false;
	int z = 0;

	Transform() = default;
	Transform(SDL_FPoint p, int z, int flip = 0, bool camera = false);
	Transform(float x, float y, int z, int flip = 0, bool camera = false);
	~Transform() = default;
};
