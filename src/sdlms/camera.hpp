#ifndef HEADER_WORLDMAP_CAMERA
#define HEADER_WORLDMAP_CAMERA

#include <SDL2/SDL.h>
#include "util/currenton.hpp"

class Camera : public Currenton<Camera>
{
public:
  Camera();

public:
  SDL_Rect viewport{0, 0, 800, 600};

private:
  Camera(const Camera &) = delete;
  Camera &operator=(const Camera &) = delete;
};

#endif