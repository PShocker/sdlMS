#include "UISkill.h"
#include "Resources/Wz.h"
#include "Button.h"

void UISkill::run()
{
    over();
}

void UISkill::show()
{
    UISkill::x = Camera::w / 2;
    UISkill::y = Camera::h / 2;

    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"UIWindow.img/Skill/backgrnd")));

    Button::load(u"UIWindow.img/KeyConfig/BtClose", BtClose);

    UISkill::open = true;
}

void UISkill::hide()
{
    UISkill::open = false;
}

void UISkill::over()
{
    for (auto &[key, val] : UISkill::position_map)
    {
        auto rect = val;
        rect.x += UISkill::x;
        rect.y += UISkill::y;
        Button::over(rect, *key);
    }
}

void UISkill::click()
{
    for (auto &[key, val] : UISkill::position_map)
    {
        auto rect = val;
        rect.x += UISkill::x;
        rect.y += UISkill::y;
        Button::click(rect, *key, UISkill::click_map);
    }
}

void UISkill::BtClose_func()
{
    UISkill::hide();
}
