module;

#include "wz/Property.hpp"
#include <SDL3/SDL.h>

export module components:skill;

import :animated;

export struct Skill
{
    struct Info
    {
        int mpCon;
        int damage;
        int mobCount;
        int attackCount;
        std::u16string action;
        SDL_FPoint lt;
        SDL_FPoint rb;
    };

    static inline std::unordered_map<std::u16string, std::vector<Animated *>> effects;
    std::vector<bool> animated;

    int level = 30;
    std::vector<Info> infos;
    std::u16string id;

    static void init();
    Skill(const std::u16string &id);
    Skill() = default;
};
