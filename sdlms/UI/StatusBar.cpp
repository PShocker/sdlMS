#include "StatusBar.h"
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
    SDL_FRect rect = {0, 0, 43, 34};
    SDL_FPoint point = {mouse_x, mouse_y};
    rect.x = 570;
    rect.y = Camera::h - 34;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        StatusBar::BtShop.first = u"mouseOver";
    }
    else
    {
        StatusBar::BtShop.first = u"normal";
    }
    rect.x += 45;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        StatusBar::BtChat.first = u"mouseOver";
    }
    else
    {
        StatusBar::BtChat.first = u"normal";
    }
    rect.x += 45;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        StatusBar::BtNPT.first = u"mouseOver";
    }
    else
    {
        StatusBar::BtNPT.first = u"normal";
    }
    rect.x += 45;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        StatusBar::BtMenu.first = u"mouseOver";
    }
    else
    {
        StatusBar::BtMenu.first = u"normal";
    }
    rect.x += 45;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        StatusBar::BtShort.first = u"mouseOver";
    }
    else
    {
        StatusBar::BtShort.first = u"normal";
    }
}

void StatusBar::init()
{
    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd")));
    backgrnd2 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd2")));
    quickSlot = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/quickSlot")));
    chatTarget = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/chatTarget")));

    graduation = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/graduation")));
    bar = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/bar")));
    gray = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/gray")));

    auto node = ui_node->find_from_path(u"StatusBar.img/BtShop");
    {
        std::unordered_map<std::u16string, AnimatedSprite> a;
        for (auto &[key, val] : node->get_children())
        {
            a[key] = AnimatedSprite(val[0]);
            BtShop.first = key;
        }
        BtShop.second = a;
    }
    node = ui_node->find_from_path(u"StatusBar.img/BtChat");
    {
        std::unordered_map<std::u16string, AnimatedSprite> a;

        for (auto &[key, val] : node->get_children())
        {
            a[key] = AnimatedSprite(val[0]);
            BtChat.first = key;
        }
        BtChat.second = a;
    }
    node = ui_node->find_from_path(u"StatusBar.img/BtNPT");
    {
        std::unordered_map<std::u16string, AnimatedSprite> a;
        for (auto &[key, val] : node->get_children())
        {
            a[key] = AnimatedSprite(val[0]);
            BtNPT.first = key;
        }
        BtNPT.second = a;
    }

    node = ui_node->find_from_path(u"StatusBar.img/BtMenu");
    {
        std::unordered_map<std::u16string, AnimatedSprite> a;
        for (auto &[key, val] : node->get_children())
        {
            a[key] = AnimatedSprite(val[0]);
            BtMenu.first = key;
        }
        BtMenu.second = a;
    }

    node = ui_node->find_from_path(u"StatusBar.img/BtShort");
    {
        std::unordered_map<std::u16string, AnimatedSprite> a;
        for (auto &[key, val] : node->get_children())
        {
            a[key] = AnimatedSprite(val[0]);
            BtShort.first = key;
        }
        BtShort.second = a;
    }
}
