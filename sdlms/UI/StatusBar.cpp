#include "StatusBar.h"
#include "Cursor.h"
#include "wz/Property.hpp"
#include "Resources/Wz.h"
#include "Components/Components.h"

void StatusBar::run()
{
    over();
}

void StatusBar::over()
{
    float mouse_x = Window::mouse_x;
    float mouse_y = Window::mouse_y;

    // 判断鼠标是否滑动到按钮附近
    SDL_FPoint point = {mouse_x, mouse_y};
    auto over_func = [&point](SDL_FRect &rect, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair)
    {
        if (SDL_PointInRectFloat(&point, &rect))
        {
            // 判断鼠标左键是否按下
            if (Cursor::left_mouse_press)
            {
                pair.first = u"pressed";
            }
            else
            {
                pair.first = u"mouseOver";
            }
        }
        else
        {
            pair.first = u"normal";
        }
    };
    for (auto &[key, val] : StatusBar::position_map)
    {
        auto rect = val;
        rect.y += Camera::h;
        over_func(rect, *key);
    }
}

void StatusBar::init()
{
    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd")));
    backgrnd2 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd2")));
    quickSlot = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/quickSlot")));
    chatTarget = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/chatTarget")));
    iconMemo = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/iconMemo")));
    box = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/box")));
    chat = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/chat")));

    graduation = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/graduation")));
    bar = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/bar")));
    gray = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/gray")));

    auto load_func = [&ui_node](std::u16string path, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair)
    {
        auto node = ui_node->find_from_path(path);
        std::unordered_map<std::u16string, AnimatedSprite> a;
        for (auto &[key, val] : node->get_children())
        {
            a[key] = AnimatedSprite(val[0]);
        }
        pair.first = u"normal";
        pair.second = a;
    };
    load_func(u"StatusBar.img/BtShop", BtShop);
    load_func(u"StatusBar.img/BtChat", BtChat);
    load_func(u"StatusBar.img/BtNPT", BtNPT);
    load_func(u"StatusBar.img/BtMenu", BtMenu);
    load_func(u"StatusBar.img/BtShort", BtShort);
    load_func(u"StatusBar.img/BtClaim", BtClaim);
    load_func(u"StatusBar.img/EquipKey", EquipKey);
    load_func(u"StatusBar.img/InvenKey", InvenKey);
    load_func(u"StatusBar.img/StatKey", StatKey);
    load_func(u"StatusBar.img/SkillKey", SkillKey);
    load_func(u"StatusBar.img/KeySet", KeySet);
    load_func(u"StatusBar.img/QuickSlot", QuickSlot);
    load_func(u"StatusBar.img/QuickSlotD", QuickSlotD);

    auto node = ui_node->find_from_path(u"Basic.img/LevelNo");
    for (int i = 0; i < 10; i++)
    {
        LevelNo[i] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(std::to_string(i))));
    }

    FreeType::size(12);
    name = FreeType::load(Player::name, SDL_Color{255, 255, 255, 255}, 0);

    FreeType::size(12);
    job = FreeType::load(u"全职业精通", SDL_Color{255, 255, 255, 255}, 0);

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
    float mouse_x = Window::mouse_x;
    float mouse_y = Window::mouse_y;

    // 判断鼠标是否滑动到按钮附近
    SDL_FPoint point = {mouse_x, mouse_y};

    auto click_func = [&point](SDL_FRect &rect, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair)
    {
        if (SDL_PointInRectFloat(&point, &rect))
        {
            if (StatusBar::click_map.contains(&pair))
            {
                auto func = StatusBar::click_map.at(&pair);
                func();
            }
        }
    };
    for (auto &[key, val] : StatusBar::position_map)
    {
        auto rect = val;
        rect.y += Camera::h;
        click_func(rect, *key);
    }
}

void StatusBar::QuickSlot_func()
{
    StatusBar::alpha += 20;
    StatusBar::alpha = std::min(StatusBar::alpha, 255);
}

void StatusBar::QuickSlotD_func()
{
    StatusBar::alpha -= 20;
    StatusBar::alpha = std::max(StatusBar::alpha, 40);
}
