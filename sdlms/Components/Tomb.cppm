module;

export module components:tomb;

import :animatedsprite;
import :transform;

export struct Tomb
{
    static inline AnimatedSpriteWarp *fall;
    static inline AnimatedSpriteWarp *land;

    Transform f;
    Transform l;

    AnimatedSprite aspr = AnimatedSprite(fall);

    static void init();
    Tomb() = default;
};
