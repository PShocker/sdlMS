#include "sdlms/camera.hpp"
#include "sdlms/map.hpp"

Camera::Camera()
{
    _viewport = SDL_FRect{0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT};
    _character = Character::current();
}

void Camera::update(int elapsedTime)
{
    Point<float> player_pos = _character->_pos - Point<float>{_viewport.w / 2, _viewport.h / 2};
    Point<float> camera_pos{_viewport.x, _viewport.y};

    _viewport.x = player_pos.x();

    _viewport.y = player_pos.y();

    auto [Left, Right, Top, Bottom] = Map::current()->_border;

    if (_viewport.x < Left)
    {
        _viewport.x = Left;
    }
    else if (_viewport.x + _viewport.w > Right)
    {
        _viewport.x = Right - _viewport.w;
    }

    if (_viewport.y + _viewport.h > Bottom)
    {
        _viewport.y = Bottom - _viewport.h;
    }
    return;
}