#pragma once
#include "Transform.h"
#include "AnimatedSprite.h"
#include "Core/Core.h"

#include <string>

struct Effect
{
    struct Wrap
    {
        std::optional<Transform> tr;
        AnimatedSprite aspr;
        unsigned int delay = Window::dt_now;
        // 是否是相对的偏移，只有在tr有值的时候才有效
        bool follow = false;
        // 摧毁时间，如果该值存在，则循环播放到这个时间后销毁
        std::optional<unsigned int> destory;
    };

    static inline const int Dizzy = 1;
    static inline const int Poison = 2;
    static inline const int Flame = 3;

    std::multimap<int, Wrap> effects;

    Effect() = default;

    static AnimatedSprite::Wrap *load(const std::u16string &path);

    static void push(Effect *eff, AnimatedSprite::Wrap *asprw, std::optional<SDL_FPoint> p, int flip);
};
