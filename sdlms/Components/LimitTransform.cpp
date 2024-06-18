#include "LimitTransform.h"

LimitTransform::LimitTransform(Transform *tr, std::optional<SDL_FPoint> h, std::optional<SDL_FPoint> v) : tr(tr), h(h), v(v)
{
}