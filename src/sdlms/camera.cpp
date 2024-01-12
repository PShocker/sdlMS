#include "camera.hpp"

Camera::Camera()
{
    viewport = SDL_Rect{0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT};
}
