#include "Transform.h"

Transform::Transform(SDL_FPoint p, int flip, bool camera):position(p), rotation(0), flip(0){}

Transform::Transform(float x, float y, int flip, bool camera):position({x, y}), rotation(0), flip(flip), camera(camera){}
