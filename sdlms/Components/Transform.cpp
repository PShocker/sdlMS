#include "Transform.h"

Transform::Transform(SDL_FPoint p, int z, int flip, bool camera) : position(p), z(z), rotation(0), flip(0) {}

Transform::Transform(float x, float y, int z, int flip, bool camera) : position({x, y}), z(z), rotation(0), flip(flip), camera(camera) {}
