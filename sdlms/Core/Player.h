#pragma once

#include "entt/entt.hpp"

// player component
struct Player
{
    static inline entt::entity ent;
    static inline int hp = 3000;
    static inline int max_hp = 3000;
    static inline int mp = 50;
    static inline int max_mp = 100;
    static inline int exp = 70;
    static inline int max_exp = 100;
    static inline int level = 100;
    static inline std::u16string name = u"\u9006\u5411\u840C\u65B0";
};
