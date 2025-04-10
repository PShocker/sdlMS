#include "StatusBar.h"
#include "KeyConfig.h"
#include "UIItem.h"
#include "UISkill.h"
#include "UIStat.h"
#include "UIEquip.h"
#include "MiniMap.h"
#include "QuickSlot.h"
#include "Button.h"
#include "wz/Property.hpp"
#include "Resources/Wz.h"
#include "Components/Components.h"

void StatusBar::run()
{
}

void StatusBar::over()
{
    for (auto &[key, val] : StatusBar::position_map)
    {
        auto rect = val;
        rect.y += Camera::h;
        Button::over(rect, *key);
    }
}

void StatusBar::init()
{
    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd")));
    backgrnd2 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd2")));
    chatTarget = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/chatTarget")));
    iconMemo = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/iconMemo")));
    box = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/box")));
    chat = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/chat")));

    graduation = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/graduation")));
    bar = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/bar")));
    gray = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/gray")));

    Button::load(u"StatusBar.img/BtShop", BtShop);
    Button::load(u"StatusBar.img/BtChat", BtChat);
    Button::load(u"StatusBar.img/BtNPT", BtNPT);
    Button::load(u"StatusBar.img/BtMenu", BtMenu);
    Button::load(u"StatusBar.img/BtShort", BtShort);
    Button::load(u"StatusBar.img/BtClaim", BtClaim);
    Button::load(u"StatusBar.img/EquipKey", EquipKey);
    Button::load(u"StatusBar.img/InvenKey", InvenKey);
    Button::load(u"StatusBar.img/StatKey", StatKey);
    Button::load(u"StatusBar.img/SkillKey", SkillKey);
    Button::load(u"StatusBar.img/KeySet", KeySet);
    Button::load(u"StatusBar.img/QuickSlot", QuickSlot);
    Button::load(u"StatusBar.img/QuickSlotD", QuickSlotD);

    auto node = ui_node->find_from_path(u"Basic.img/LevelNo");
    for (int i = 0; i < 10; i++)
    {
        LevelNo[i] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(std::to_string(i))));
    }

    name = FreeType::load(Player::name, SDL_Color{255, 255, 255, 255}, 0, 12);

    job = FreeType::load(u"\u5168\u804C\u4E1A\u7CBE\u901A", SDL_Color{255, 255, 255, 255}, 0, 12);

    node = ui_node->find_from_path(u"StatusBar.img/number");
    for (int i = 0; i < 10; i++)
    {
        number[i] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(std::to_string(i))));
    }
    number[10] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"Lbracket")));
    number[11] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"Rbracket")));
    number[12] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"slash")));
    number[13] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"percent")));
}

void StatusBar::click()
{
    for (auto &[key, val] : StatusBar::position_map)
    {
        auto rect = val;
        rect.y += Camera::h;
        Button::click(rect, *key, StatusBar::click_map);
    }
}

void StatusBar::QuickSlot_func()
{
    StatusBar::alpha += 20;
    StatusBar::alpha = StatusBar::alpha > 255 ? 40 : StatusBar::alpha;
    KeyConfig::alpha = StatusBar::alpha;
    UIItem::alpha = StatusBar::alpha;
    MiniMap::alpha = StatusBar::alpha;
    QuickSlot::alpha = StatusBar::alpha;
}

void StatusBar::QuickSlotD_func()
{
    StatusBar::alpha -= 20;
    StatusBar::alpha = StatusBar::alpha < 40 ? 255 : StatusBar::alpha;
    KeyConfig::alpha = StatusBar::alpha;
    UIItem::alpha = StatusBar::alpha;
    MiniMap::alpha = StatusBar::alpha;
    QuickSlot::alpha = StatusBar::alpha;
}

void StatusBar::KeySet_func()
{
    KeyConfig::show();
}

void StatusBar::InvenKey_func()
{
    UIItem::show();
}

void StatusBar::SkillKey_func()
{
    UISkill::show();
}

void StatusBar::StatKey_func()
{
    UIStat::show();
}

void StatusBar::EquipKey_func()
{
    UIEquip::show();
}
