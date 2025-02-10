#include "Drop.h"
#include "Resources/Wz.h"

#include <stdint.h>
#include <string>

void Drop::init()
{
    mesoicons[MesoIcon::BRONZE] = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000000/iconRaw"), 255, false);
    mesoicons[MesoIcon::GOLD] = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000001/iconRaw"), 255, false);
    mesoicons[MesoIcon::BUNDLE] = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000002/iconRaw"), 255, false);
    mesoicons[MesoIcon::BAG] = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000003/iconRaw"), 255, false);
}
