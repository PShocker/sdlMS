module;

#include "entt/entt.hpp"

module systems;

import core;
import components;

void camera_run()
{
    auto ent = Player::ent;
    if (World::registry.valid(ent))
    {
        auto tr = World::registry.try_get<Transform>(ent);

        auto h_prev_x = Camera::x;                      // 上一帧摄像机坐标
        auto h_next_x = tr->position.x - Camera::w / 2; // 人物移动后新的摄像机坐标
        auto h_delta = h_next_x - h_prev_x;             // 新的摄像机坐标与旧的摄像机坐标差值

        Camera::x = std::lerp(h_prev_x, h_next_x, std::abs(h_delta) / 6000.0f);

        auto v_prev_y = Camera::y;                      // 上一帧摄像机坐标
        auto v_next_y = tr->position.y - Camera::h / 2; // 人物移动后新的摄像机坐标
        auto v_delta = v_next_y - v_prev_y;             // 新的摄像机坐标与旧的摄像机坐标差值

        Camera::y = std::lerp(v_prev_y, v_next_y, std::abs(v_delta) / 6000.0f);
    }
}