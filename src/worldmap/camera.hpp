#include <SDL2/SDL.h>
#include "util/currenton.hpp"

namespace worldmap
{

  class Camera : public Currenton<Camera>
  {
  public:
    Camera();

  private:
    SDL_Rect *rect;

  private:
    Camera(const Camera &) = delete;
    Camera &operator=(const Camera &) = delete;
  };

} // namespace worldmap
