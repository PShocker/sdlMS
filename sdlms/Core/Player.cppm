module;

#include "entt/entt.hpp"

export module core:player;

export struct Player
{
    static inline entt::entity ent;

    struct CoolDown
    {
        static inline std::tuple<int, const int, bool> foothold = {0, 120, false};
        static inline std::tuple<int, const int, bool> portal = {0, 400, false};
    };
};
