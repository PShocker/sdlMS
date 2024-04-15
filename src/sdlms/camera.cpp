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
    return;
    // int VWIDTH = _viewport.w;
    // int VHEIGHT = _viewport.h;

    // double next_x = _viewport.x;
    // double hdelta = VWIDTH / 2 - player_pos.x() - next_x;

    // if (std::abs(hdelta) >= 5.0)
    //     next_x += hdelta * (12.0 / VWIDTH);

    // double next_y = _viewport.y;
    // double vdelta = VHEIGHT / 2 - player_pos.y() - next_y;

    // if (std::abs(vdelta) >= 5.0)
    //     next_y += vdelta * (12.0 / VHEIGHT);

    // auto [Left, Right, Top, Bottom] = Map::current()->_border;

    // if (next_x < Left)
    // {
    //     next_x = Left;
    // }
    // else if (next_x + VWIDTH > Right)
    // {
    //     next_x = Right - VWIDTH;
    // }

    // if (next_y < Top)
    // {
    //     next_y = Top;
    // }
    // else if (next_y + VHEIGHT > Bottom)
    // {
    //     next_y = Bottom - VHEIGHT;
    // }
    // _viewport.x = next_x;
    // _viewport.y = next_y;

    return;
}