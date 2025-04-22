#include "Camera.h"
#include "Core/Core.h"
#include "Components/Components.h"
#include "entt/entt.hpp"

void camera_run()
{
    if (auto ent = Player::ent; World::registry->valid(ent))
    {
        auto tr = World::registry->try_get<Transform>(ent);

        auto h_prev_x = Camera::x;                      // 上一帧摄像机坐标
        auto h_next_x = tr->position.x - Camera::w / 2; // 人物移动后新的摄像机坐标
        auto h_delta = h_next_x - h_prev_x;             // 新的摄像机坐标与旧的摄像机坐标差值

        Camera::x = std::roundf(std::lerp((float)h_prev_x, h_next_x, std::abs(h_delta) / 6000.0f));

        auto v_prev_y = Camera::y;                      // 上一帧摄像机坐标
        auto v_next_y = tr->position.y - Camera::h / 2; // 人物移动后新的摄像机坐标
        auto v_delta = v_next_y - v_prev_y;             // 新的摄像机坐标与旧的摄像机坐标差值

        Camera::y = std::roundf(std::lerp((float)v_prev_y, v_next_y, std::abs(v_delta) / 6000.0f));
    }
    camera_limit();
    camera_shake();
}

void camera_refresh()
{
    if (auto ent = Player::ent; World::registry->valid(ent))
    {
        auto tr = World::registry->try_get<Transform>(ent);
        auto h_next_x = tr->position.x - Camera::w / 2; // 人物移动后新的摄像机坐标
        auto v_next_y = tr->position.y - Camera::h / 2; // 人物移动后新的摄像机坐标

        Camera::x = h_next_x;
        Camera::y = v_next_y;
    }
    camera_limit();
    Camera::shake_strength = 0;
    Camera::shake_duration = 0;
    Camera::shake_delay = 0;
}

void camera_limit()
{
    std::optional<int> l = std::nullopt;
    std::optional<int> r = std::nullopt;
    std::optional<int> t = std::nullopt;
    std::optional<int> b = std::nullopt;

    if (Camera::l.has_value() &&
        Camera::r.has_value() &&
        Camera::t.has_value() &&
        Camera::b.has_value())
    {
        l = Camera::l;
        r = Camera::r;
        t = Camera::t;
        b = Camera::b;
    }
    else
    {
        auto border = World::registry->ctx().get<Border>();
        l = border.l;
        r = border.r;
        t = border.t;
        b = border.b;
    }
    if (l.has_value() && Camera::x < l.value())
    {
        Camera::x = l.value();
    }
    else if (r.has_value() && Camera::x + Camera::w > r.value())
    {
        Camera::x = r.value() - Camera::w;
    }
    if (b.has_value() && Camera::y + Camera::h > b.value())
    {
        Camera::y = b.value() - Camera::h;
    }

    if (r.has_value() && l.has_value())
    {
        if (r.value() - l.value() < Camera::w)
        {
            // 如果地图宽度小于摄像机宽度,地图居中显示
            Camera::x = l.value() - (Camera::w - (r.value() - l.value())) / 2;
        }
    }
    if (b.has_value() && t.has_value())
    {
        if (b.value() - t.value() < Camera::h)
        {
            // 如果地图高度小于摄像机高度,地图居中显示
            Camera::y = t.value() - (Camera::h - (b.value() - t.value())) / 2;
        }
    }
}

void camera_shake()
{
    if (Camera::shake_delay > 0)
    {
        Camera::shake_delay -= Window::delta_time;
    }
    else if (Camera::shake_duration > 0)
    {
        Camera::shake_duration -= Window::delta_time;
        // 每帧进行震动，随机移动相机位置
        Camera::x += (rand() % 3 - 1) * Camera::shake_strength; // 随机抖动
        Camera::y += (rand() % 3 - 1) * Camera::shake_strength; // 随机抖动
    }
}

void camera_init_shake(float strength, int duration, int delay)
{
    Camera::shake_strength = strength;
    Camera::shake_duration = duration;
    Camera::shake_delay = delay;
}