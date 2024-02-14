#ifndef HEADER_SDLMS_CAMERA
#define HEADER_SDLMS_CAMERA

#include <SDL2/SDL.h>
#include "template/currenton.hpp"
#include "sdlms/graphics.hpp"
#include "sdlms/input.hpp"

class Camera : public Currenton<Camera>
{
public:
  Camera();
  void event(SDL_Event &event);

public:
  SDL_FRect _viewport;

private:
  Camera(const Camera &) = delete;
  Camera &operator=(const Camera &) = delete;

private:
  Input *_input;
};

#endif