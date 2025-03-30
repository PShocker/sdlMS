#include "KeyConfig.h"
#include "Resources/Wz.h"

void KeyConfig::show()
{
    KeyConfig::open = true;
}

void KeyConfig::hide()
{
    KeyConfig::open = false;
}

void KeyConfig::init()
{
    KeyConfig::x = Camera::w / 2;
    KeyConfig::y = Camera::h / 2;

    auto ui_node = Wz::UI->get_root();
    backgrnd = Sprite(ui_node->find_from_path(u"UIWindow.img/KeyConfig/backgrnd"));
}
