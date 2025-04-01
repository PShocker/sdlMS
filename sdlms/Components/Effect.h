#pragma once
#include "Transform.h"
#include "AnimatedSprite.h"
#include "Core/Core.h"

#include <string>

struct Effect
{
    static inline const std::u16string Dizzy = u"1";
    static inline const std::u16string Poison = u"2";
    static inline const std::u16string Flame = u"3";

    struct Info
    {
        std::optional<Transform> tr;
        AnimatedSprite aspr;
        unsigned int delay = Window::dt_now;
        // 是否是相对的偏移，只有在tr有值的时候才有效
        bool follow = false;
        // 摧毁时间，如果该值存在，则循环播放到这个时间后销毁
        std::optional<unsigned int> destory;
    };

    std::multimap<std::u16string, Info> effects;

    Effect() = default;

    static AnimatedSpriteWarp *load(const std::u16string &path);

    static void push(Effect *eff, AnimatedSpriteWarp *asprw, std::optional<SDL_FPoint> p, int flip);
};
