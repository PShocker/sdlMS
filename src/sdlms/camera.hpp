#ifndef HEADER_SDLMS_CAMERA
#define HEADER_SDLMS_CAMERA

#include <SDL2/SDL.h>
#include "util/currenton.hpp"
#include "sdlms/graphics.hpp"


class Camera : public Currenton<Camera>
{
public:
  Camera();

public:
  SDL_FRect viewport;

private:
  Camera(const Camera &) = delete;
  Camera &operator=(const Camera &) = delete;
};

#endif