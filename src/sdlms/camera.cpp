#include "camera.hpp"

Camera::Camera()
{
    viewport = SDL_FRect{0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT};
}
