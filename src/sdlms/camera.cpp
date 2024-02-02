#include "sdlms/camera.hpp"

Camera::Camera()
{
    _viewport = SDL_FRect{0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT};

    _input = Input::current();
    _input->event(std::bind(&handle_input, this, std::placeholders::_1));
}

void Camera::handle_input(SDL_Event &event)
{
    if (_input->isKeyHeld(SDL_SCANCODE_LEFT) == true)
    {
        _viewport.x -= 10;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_RIGHT) == true)
    {
        _viewport.x += 10;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_UP) == true)
    {
        _viewport.y -= 10;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_DOWN) == true)
    {
        _viewport.y += 10;
    }
}
