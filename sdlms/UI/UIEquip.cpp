#include "UIEquip.h"
#include "Resources/Wz.h"
#include "Button.h"
#include "Systems/UI.h"

void UIEquip::run()
{
}

void UIEquip::show()
{
    UIEquip::x = Camera::w / 2;
    UIEquip::y = Camera::h / 2;

    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"UIWindow.img/Equip/backgrnd")));

    Button::load(u"Basic.img/BtClose", BtClose);

    ui_index.push_back(UIIndex::UI_UIEquip);
    UIEquip::open = true;
}

void UIEquip::hide()
{
    ui_index.remove(UIIndex::UI_UIEquip);
    UIEquip::open = false;
}

void UIEquip::over()
{
    for (auto &[key, val] : UIEquip::position_map)
    {
        auto rect = val;
        rect.x += UIEquip::x;
        rect.y += UIEquip::y;
        Button::over(rect, *key);
    }
}

bool UIEquip::mousein()
{
    SDL_FPoint point = {Cursor::x, Cursor::y};
    SDL_FRect rect;
    rect.x = UIEquip::x;
    rect.y = UIEquip::y;
    rect.w = UIEquip::backgrnd->w;
    rect.h = UIEquip::backgrnd->h;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        return true;
    }
    return false;
}

void UIEquip::click()
{
    for (auto &[key, val] : UIEquip::position_map)
    {
        auto rect = val;
        rect.x += UIEquip::x;
        rect.y += UIEquip::y;
        Button::click(rect, *key, UIEquip::click_map);
    }
}

void UIEquip::BtClose_func()
{
    UIEquip::hide();
}
