#pragma once

#include "entt/entt.hpp"

// player component
struct Player
{
    static inline entt::entity ent;
    static inline int hp = 2400;
    static inline int max_hp = 3000;
    static inline int mp = 3000;
    static inline int max_mp = 3000;
    static inline int exp = 70;
    static inline int max_exp = 100;
    static inline int level = 100;
    static inline std::u16string name = u"逆向萌新";
};
