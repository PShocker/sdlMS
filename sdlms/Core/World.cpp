module;

#include "entt/entt.hpp"

module core;

import components;

void Transform_construct(entt::registry &registry, entt::entity entity)
{
    // 排序实体，假设根据 Transform 组件的 z 值排序
    registry.sort<Transform>([](const auto &lhs, const auto &rhs){return lhs.z < rhs.z;});
}

void World::init()
{
    registry.on_construct<Transform>().connect<&Transform_construct>();
}