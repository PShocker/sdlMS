module;

#include "entt/entt.hpp"

export module core:player;

// player component
export struct Player
{
    static inline entt::entity ent;

    struct CoolDown
    {
        int current;
        int delay;
        bool start;
    };

    struct CoolDowns
    {
        static inline CoolDown foothold{0, 120, false};
        static inline CoolDown portal{0, 400, false};
    };
};
