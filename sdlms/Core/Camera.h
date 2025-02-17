#pragma once

#include <optional>

struct Camera
{
    static inline int x = 0;
    static inline int y = 0;
    static inline int w = 0;
    static inline int h = 0;

    static inline std::optional<int> l = std::nullopt;
    static inline std::optional<int> r = std::nullopt;
    static inline std::optional<int> t = std::nullopt;
    static inline std::optional<int> b = std::nullopt;

    static inline float shake_strength = 0; // 震动强度
    static inline int shake_duration = 0;   // 震动持续时间
    static inline int shake_delay = 0;
};