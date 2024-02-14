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
  void event(SDL_Event &event);

public:
  SDL_FRect _viewport;

private:
  // 相机最小移动距离，若小于此距离，则移动这个最小距离的长度
  const float CAMERA_MIN_MOVE_DISTANCE = 0.1;
  // 每帧逼近剩余距离的百分比
  const float CAMERA_MOVE_INTERPOLATE = 0.05;

private:
  Camera(const Camera &) = delete;
  Camera &operator=(const Camera &) = delete;

private:
  Input *_input;
  // Character *_character;
};

#endif