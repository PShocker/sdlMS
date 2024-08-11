module;

module components;

import :animatedsprite;
import resources;

void Tomb::init()
{
    fall = AnimatedSpriteWarp::load(Wz::Effect->get_root()->find_from_path(u"Tomb.img/fall"));
    land = AnimatedSpriteWarp::load(Wz::Effect->get_root()->find_from_path(u"Tomb.img/land"));
}
