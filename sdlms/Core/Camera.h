#pragma once

struct Camera
{
    static inline int x = 0;
    static inline int y = 0;
    static inline int w = 0;
    static inline int h = 0;

    static inline float shake_strength = 0; // 震动强度
    static inline int shake_duration = 0;   // 震动持续时间
    static inline int shake_delay = 0;
};