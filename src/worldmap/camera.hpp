#include <SDL2/SDL.h>

namespace worldmap
{

  class Camera
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
