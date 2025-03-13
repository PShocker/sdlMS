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
        if (p.has_value())
        {
            eff->effects.push_back({new Transform(p.value(), 0, flip),
                                    AnimatedSprite(asprw), Window::dt_now});
        }
        else
        {
            eff->effects.push_back({nullptr, AnimatedSprite(asprw), Window::dt_now});
        }
    }
}