#include "Effect.h"
#include "Resources/Wz.h"
#include "wz/Property.hpp"

AnimatedSpriteWarp *Effect::load(const std::u16string &path)
{
    auto node = Wz::Effect->get_root()->find_from_path(path);
    return AnimatedSpriteWarp::load(node);
}

void Effect::push(Effect *eff, AnimatedSpriteWarp *asprw, std::optional<SDL_FPoint> p, int flip)
{
    if (asprw)
    {
        std::optional<Transform> tr = std::nullopt;
        if (p.has_value())
        {
            tr = Transform(p.value());
        }
        eff->effects.insert({0, {tr, AnimatedSprite(asprw)}});
    }
}