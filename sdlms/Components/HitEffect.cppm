module;

export module components:hiteffect;
import :animatedsprite;
import :animatedsprite;

export struct HitEffect
{
    AnimatedSprite *effect;
    int damage;

    float x;
    float y;

    bool hit=false;

    HitEffect(AnimatedSpriteWarp *aspr);
    HitEffect() = default;
    ~HitEffect();
};