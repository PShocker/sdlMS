#include "Effect.h"
#include "Resources/Wz.h"
#include "wz/Property.hpp"

AnimatedSprite::Wrap *Effect::load(const std::u16string &path)
{
    auto node = Wz::Effect->get_root()->find_from_path(path);
    return AnimatedSprite::Wrap::load(node);
}

void Effect::push(Effect *eff, AnimatedSprite::Wrap *asprw, std::optional<SDL_FPoint> position, int flip, bool target)
{
    if (asprw)
    {
        std::optional<Transform> tr = std::nullopt;
        if (position.has_value())
        {
            tr = Transform(position.value());
        }
        eff->effects.insert({0, {tr, AnimatedSprite(asprw), Window::dt_now, false, std::nullopt, target}});
    }
}