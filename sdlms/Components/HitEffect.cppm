module;

export module components:hiteffect;
import :animatedsprite;
import :animatedsprite;

export struct HitEffect
{
    AnimatedSprite* effect;
    HitEffect(AnimatedSpriteWarp* aspr);
    HitEffect() = default;
};