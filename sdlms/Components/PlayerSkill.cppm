module;

#include <string>

export module components:playerskill;
import :skill;
import :animated;

export struct PlayerSkill
{
    Skill *ski = nullptr;
    std::vector<bool> animated;
    std::vector<Animated> effects;
    int level = 29;

    PlayerSkill(const std::u16string &id);
    PlayerSkill() = default;
};
