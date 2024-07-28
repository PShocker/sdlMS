module;

export module components:hiteffect;
import :animated;
import :animatedsprite;

export struct HitEffect
{
    Animated effect;
    HitEffect(AnimatedSprite* aspr);
    HitEffect() = default;
};

