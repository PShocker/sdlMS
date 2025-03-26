#include "StatusBar.h"
#include "wz/Property.hpp"
#include "Resources/Wz.h"
#include "Components/Components.h"

void StatusBar::init()
{
    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd")));
    backgrnd2 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd2")));
    quickSlot = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/quickSlot")));
    chatTarget = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/chatTarget")));

    auto node = ui_node->find_from_path(u"StatusBar.img/BtShop");
    for (auto &[key, val] : node->get_children())
    {
        BtShop[key] = AnimatedSprite(val[0]);
    }
    node = ui_node->find_from_path(u"StatusBar.img/BtChat");
    for (auto &[key, val] : node->get_children())
    {
        BtChat[key] = AnimatedSprite(val[0]);
    }
    node = ui_node->find_from_path(u"StatusBar.img/BtNPT");
    for (auto &[key, val] : node->get_children())
    {
        BtNPT[key] = AnimatedSprite(val[0]);
    }
    node = ui_node->find_from_path(u"StatusBar.img/BtMenu");
    for (auto &[key, val] : node->get_children())
    {
        BtMenu[key] = AnimatedSprite(val[0]);
    }
    node = ui_node->find_from_path(u"StatusBar.img/BtShort");
    for (auto &[key, val] : node->get_children())
    {
        BtShort[key] = AnimatedSprite(val[0]);
    }
}
