module;

#include <stdint.h>
#include <string>

module components;

import :animatedsprite;
import resources;

void Drop::init()
{
    mesoicons[MesoIcon::BRONZE] = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000000/iconRaw"));
    mesoicons[MesoIcon::GOLD] = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000001/iconRaw"));
    mesoicons[MesoIcon::BUNDLE] = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000002/iconRaw"));
    mesoicons[MesoIcon::BAG] = AnimatedSpriteWarp::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000003/iconRaw"));
}
