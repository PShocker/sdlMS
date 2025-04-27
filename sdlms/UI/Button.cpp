#include "Button.h"
#include "Cursor.h"
#include "Resources/Wz.h"

void Button::load(std::u16string path, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair)
{
    auto ui_node = Wz::UI->get_root();
    auto node = ui_node->find_from_path(path);
    std::unordered_map<std::u16string, AnimatedSprite> a;
    for (auto &[key, val] : node->get_children())
    {
        a[key] = AnimatedSprite(val[0]);
    }
    pair.first = u"normal";
    pair.second = a;
}

void Button::over(SDL_FRect &rect, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair)
{
    // 判断鼠标是否滑动到按钮附近
    SDL_FPoint point = {Cursor::x, Cursor::y};
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
}

void Button::click(SDL_FRect &rect, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair, std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, std::function<void()>> click_map)
{
    SDL_FPoint point = {Cursor::x, Cursor::y};

    if (SDL_PointInRectFloat(&point, &rect))
    {
        if (click_map.contains(&pair))
        {
            auto func = click_map.at(&pair);
            func();
            pair.first = u"normal";
        }
    }
}
