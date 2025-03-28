#pragma once

#include "entt/entt.hpp"

// player component
struct Player
{
    static inline entt::entity ent;
    static inline int hp = 3000;
    static inline int max_hp = 3000;
    static inline int mp;
    static inline int max_mp;
    static inline int level = 100;
    static inline std::u16string name = u"逆向萌新";
};
