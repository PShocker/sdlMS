#include "sdlms/camera.hpp"

Camera::Camera()
{
    _viewport = SDL_FRect{0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT};

    _input = Input::current();
    _input->event(std::bind(&event, this, std::placeholders::_1));
}

void Camera::event(SDL_Event &event)
{
    Point<float> p{};
}
