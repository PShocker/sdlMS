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

    auto hdelta = player_pos.x() - camera_pos.x();
    if (std::abs(hdelta) >= 5.0)
    {
        _viewport.x += hdelta * (12.0 / _viewport.w);
    }

    auto vdelta = player_pos.y() - camera_pos.y();
    if (std::abs(vdelta) >= 5.0)
    {
        _viewport.y += vdelta * (12.0 / _viewport.h);
    }

    auto [Left, Right, Top, Bottom] = Map::current()->_border;

    if (_viewport.x < Left)
    {
        _viewport.x = Left;
    }
    else if (_viewport.x + _viewport.w > Right)
    {
        _viewport.x = Right - _viewport.w;
    }

    else if (_viewport.y + _viewport.h > Bottom)
    {
        _viewport.y = Bottom - _viewport.h;
    }
    else if (_viewport.y < Top)
    {
        _viewport.y = Top;
    }

    return;
}