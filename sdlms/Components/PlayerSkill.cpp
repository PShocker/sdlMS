module;
#include <string>
module components;

PlayerSkill::PlayerSkill(const std::u16string &id)
{
    ski = load_skill(id);
    for (auto &it : ski->effects)
    {
        effects.push_back(Animated(it));
    }

    animated.resize(ski->effects.size(), false);
}