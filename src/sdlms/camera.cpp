#include "sdlms/camera.hpp"

Camera::Camera()
{
    _viewport = SDL_FRect{0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT};

    _input = Input::current();
    _input->event(std::bind(&event, this, std::placeholders::_1));

    _character = Character::current();
}

void Camera::event(SDL_Event &event)
{
    Point<float> player_pos = _character->_pos - Point<float>{_viewport.w / 2, _viewport.h / 2};
    Point<float> camera_pos{_viewport.x, _viewport.y};

    // _viewport.x = player_pos.a;
    // _viewport.y = player_pos.b;

    if (camera_pos.distance(player_pos) < CAMERA_MIN_MOVE_DISTANCE)
    {
        _viewport.x = player_pos.a;
        _viewport.y = player_pos.b;
    }
    else
    {
        // 相机下一帧位置
        auto camera_next_pos = camera_pos + (player_pos - camera_pos) * CAMERA_MOVE_INTERPOLATE;
        _viewport.x = camera_next_pos.a;
        _viewport.y = camera_next_pos.b;
    }
    return;
}
