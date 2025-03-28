#include "Tomb.h"
#include "Resources/Wz.h"

void Tomb::init()
{
    fall = AnimatedSpriteWarp::load(Wz::Effect->get_root()->find_from_path(u"Tomb.img/fall"), 255);
    land = AnimatedSpriteWarp::load(Wz::Effect->get_root()->find_from_path(u"Tomb.img/land"), 255);
}
