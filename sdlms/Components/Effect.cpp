module;

#include "wz/Property.hpp"

module components;
import resources;

AnimatedSpriteWarp *Effect::load(const std::u16string &path)
{
    auto node = Wz::Effect->get_root()->find_from_path(path);
    return AnimatedSpriteWarp::load(node);
}
