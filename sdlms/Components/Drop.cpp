#include "Drop.h"
#include "Resources/Wz.h"

#include <stdint.h>
#include <string>

void Drop::init()
{
    mesoicons[MesoIcon::BRONZE] = AnimatedSprite::Wrap::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000000/iconRaw"));
    mesoicons[MesoIcon::GOLD] = AnimatedSprite::Wrap::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000001/iconRaw"));
    mesoicons[MesoIcon::BUNDLE] = AnimatedSprite::Wrap::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000002/iconRaw"));
    mesoicons[MesoIcon::BAG] = AnimatedSprite::Wrap::load(Wz::Item->get_root()->find_from_path(u"Special/0900.img/09000003/iconRaw"));
}
