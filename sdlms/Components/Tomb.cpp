#include "Tomb.h"
#include "Resources/Wz.h"

void Tomb::init()
{
    fall = AnimatedSprite::Wrap::load(Wz::Effect->get_root()->find_from_path(u"Tomb.img/fall"));
    land = AnimatedSprite::Wrap::load(Wz::Effect->get_root()->find_from_path(u"Tomb.img/land"));
}
