#pragma once
#include "entt/entt.hpp"
#include "Core/Core.h"

struct Summon
{
    std::u16string id;
    entt::entity owner = entt::null;

    // 摧毁时间
    unsigned int destory = Window::dt_now + 30000;
};