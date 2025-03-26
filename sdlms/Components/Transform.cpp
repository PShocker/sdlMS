#include "Transform.h"

#include <SDL3/SDL.h>

Transform::Transform(SDL_FPoint p, int z, int flip) : position(p), z(z), rotation(0), flip(flip) {}
Transform::Transform(float x, float y, int z, int flip) : position({x, y}), z(z), rotation(0), flip(flip) {}
