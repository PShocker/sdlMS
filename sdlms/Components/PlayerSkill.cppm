module;

#include <string>

export module components:playerskill;
import :skill;
import :animatedsprite;

export struct PlayerSkill
{
    Skill *ski = nullptr;
    std::vector<bool> animated;
    std::vector<AnimatedSprite *> effects;
    int level = 29;

    PlayerSkill(const std::u16string &id);
    PlayerSkill() = default;
    ~PlayerSkill();
};
