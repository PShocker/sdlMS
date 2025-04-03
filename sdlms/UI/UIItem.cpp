#include "UIItem.h"
#include "Resources/Wz.h"
#include "Button.h"
#include "Systems/UI.h"

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

    ui_index.push_back(UIIndex::UI_UIItem);
    UIItem::open = true;
}

void UIItem::hide()
{
    ui_index.remove(UIIndex::UI_UIItem);
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

bool UIItem::mousein()
{
    float mouse_x = Window::mouse_x;
    float mouse_y = Window::mouse_y;

    SDL_FPoint point = {mouse_x, mouse_y};
    SDL_FRect rect;
    rect.x = UIItem::x;
    rect.y = UIItem::y;
    rect.w = UIItem::FullBackgrnd->w;
    rect.h = UIItem::FullBackgrnd->h;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        return true;
    }
    return false;
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
