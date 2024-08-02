module;
#include <string>
module components;
import :skill;

PlayerSkill::PlayerSkill(const std::u16string &id)
{
    ski = Skill::load(id);
    for (auto &it : ski->effects)
    {
        effects.push_back(new AnimatedSprite(it));
    }
    animated.resize(ski->effects.size(), false);
}

PlayerSkill::~PlayerSkill()
{
    for (auto it : effects){
        delete it;
    }
}
