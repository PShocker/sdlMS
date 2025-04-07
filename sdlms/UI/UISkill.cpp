#include "UISkill.h"
#include "Resources/Wz.h"
#include "Button.h"
#include "Systems/UI.h"

void UISkill::run()
{
}

void UISkill::show()
{
    UISkill::x = Camera::w / 2;
    UISkill::y = Camera::h / 2;

    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"UIWindow.img/Skill/backgrnd")));

    Button::load(u"Basic.img/BtClose", BtClose);

    ui_index.push_back(UIIndex::UI_UISkill);
    UISkill::open = true;
}

void UISkill::hide()
{
    ui_index.remove(UIIndex::UI_UISkill);
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

bool UISkill::mousein()
{
    SDL_FPoint point = {Cursor::x, Cursor::y};
    SDL_FRect rect;
    rect.x = UISkill::x;
    rect.y = UISkill::y;
    rect.w = UISkill::backgrnd->w;
    rect.h = UISkill::backgrnd->h;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        return true;
    }
    return false;
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
