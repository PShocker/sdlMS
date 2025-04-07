#include "KeyConfig.h"
#include "Resources/Wz.h"
#include "Button.h"
#include "Systems/UI.h"

void KeyConfig::run()
{
}

void KeyConfig::show()
{
    KeyConfig::x = Camera::w / 2;
    KeyConfig::y = Camera::h / 2;

    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"UIWindow.img/KeyConfig/backgrnd")));

    Button::load(u"Basic.img/BtClose", BtClose);

    ui_index.push_back(UIIndex::UI_KeyConfig);
    KeyConfig::open = true;
}

void KeyConfig::hide()
{
    ui_index.remove(UIIndex::UI_KeyConfig);
    KeyConfig::open = false;
}

void KeyConfig::over()
{
    for (auto &[key, val] : KeyConfig::position_map)
    {
        auto rect = val;
        rect.x += KeyConfig::x;
        rect.y += KeyConfig::y;
        Button::over(rect, *key);
    }
}

bool KeyConfig::mousein()
{
    SDL_FPoint point = {Cursor::x, Cursor::y};
    SDL_FRect rect;
    rect.x = KeyConfig::x;
    rect.y = KeyConfig::y;
    rect.w = KeyConfig::backgrnd->w;
    rect.h = KeyConfig::backgrnd->h;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        return true;
    }
    return false;
}

void KeyConfig::click()
{
    for (auto &[key, val] : KeyConfig::position_map)
    {
        auto rect = val;
        rect.x += KeyConfig::x;
        rect.y += KeyConfig::y;
        Button::click(rect, *key, KeyConfig::click_map);
    }
}

void KeyConfig::BtClose_func()
{
    KeyConfig::hide();
}
