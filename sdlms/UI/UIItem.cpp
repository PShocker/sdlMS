#include "UIItem.h"
#include "Resources/Wz.h"
#include "Button.h"

void UIItem::run()
{
    over();
}

void UIItem::show()
{
    UIItem::x = Camera::w / 2;
    UIItem::y = Camera::h / 2;

    auto ui_node = Wz::UI->get_root();
    FullBackgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"UIWindow.img/Item/FullBackgrnd")));

    Button::load(u"UIWindow.img/KeyConfig/BtClose", BtClose);

    UIItem::open = true;
}

void UIItem::hide()
{
    UIItem::open = false;
}

void UIItem::over()
{
    for (auto &[key, val] : UIItem::position_map)
    {
        auto rect = val;
        rect.x += UIItem::x;
        rect.y += UIItem::y;
        Button::over(rect, *key);
    }
}

void UIItem::click()
{
    for (auto &[key, val] : UIItem::position_map)
    {
        auto rect = val;
        rect.x += UIItem::x;
        rect.y += UIItem::y;
        Button::click(rect, *key, UIItem::click_map);
    }
}

void UIItem::BtClose_func()
{
    UIItem::hide();
}
