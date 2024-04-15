#ifndef HEADER_SDLMS_CAMERA
#define HEADER_SDLMS_CAMERA

#include <SDL2/SDL.h>

#include "template/point.hpp"
#include "template/currenton.hpp"

#include "sdlms/graphics.hpp"
#include "sdlms/input.hpp"
#include "sdlms/character.hpp"


class Camera : public Currenton<Camera>
{
public:
  Camera();
  void update(int elapsedTime);

public:
  SDL_FRect _viewport;

private:
  Camera(const Camera &) = delete;
  Camera &operator=(const Camera &) = delete;

private:
  Input *_input;
  Character *_character;
};

#endif