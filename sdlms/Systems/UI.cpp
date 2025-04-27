#include "UI.h"

std::list<UIIndex> ui_index = {UIIndex::UI_UIBuff, UIIndex::UI_StatusBar, UIIndex::UI_QuickSlot, UIIndex::UI_MiniMap};

void ui_run()
{
    auto index = Cursor::index;
    Cursor::index = u"0";
    ToolTip::run();
    MiniMap::run();
    UIBuff::run();
    GainTip::run();
    StatusBar::run();
    // 检测左键是否被按住
    if (Window::mouse_state & SDL_BUTTON_LMASK)
    {
        Cursor::left_mouse_press = true;
        Cursor::index = u"12";
        Cursor::drag = ui_drag();
        ui_sort();
    }
    else
    {
        if (Cursor::left_mouse_press)
        {
            // 播放点击的声音
            Sound::push(Sound(u"UI.img/BtMouseClick"));
            // 触发点击事件
            ui_click();
        }
        Cursor::left_mouse_press = false;
        Cursor::drag = false;
    }
    ui_keyboard();
    ui_over();
    Cursor::action(index);
    Cursor::run();
}

bool ui_drag()
{
    static float x = 0;
    static float y = 0;
    auto index = ui_index.back();
    if (index == UIIndex::UI_UIBuff || index == UIIndex::UI_StatusBar)
    {
        return false;
    }
    switch (index)
    {
    case UIIndex::UI_KeyConfig:
    {
        if (Cursor::drag)
        {
            KeyConfig::x = Cursor::x - x;
            KeyConfig::y = Cursor::y - y;
            return true;
        }
        else if (KeyConfig::mousein())
        {
            x = Cursor::x - KeyConfig::x;
            y = Cursor::y - KeyConfig::y;
            return true;
        }
    }
    break;
    case UIIndex::UI_UIItem:
    {
        if (Cursor::drag)
        {
            UIItem::x = Cursor::x - x;
            UIItem::y = Cursor::y - y;
            return true;
        }
        else if (UIItem::mousein())
        {
            x = Cursor::x - UIItem::x;
            y = Cursor::y - UIItem::y;
            return true;
        }
    }
    break;
    case UIIndex::UI_UISkill:
    {
        if (Cursor::drag)
        {
            UISkill::x = Cursor::x - x;
            UISkill::y = Cursor::y - y;
            return true;
        }
        else if (UISkill::mousein())
        {
            x = Cursor::x - UISkill::x;
            y = Cursor::y - UISkill::y;
            return true;
        }
    }
    break;
    case UIIndex::UI_UIStat:
    {
        if (Cursor::drag)
        {
            UIStat::x = Cursor::x - x;
            UIStat::y = Cursor::y - y;
            return true;
        }
        else if (UIStat::mousein())
        {
            x = Cursor::x - UIStat::x;
            y = Cursor::y - UIStat::y;
            return true;
        }
    }
    break;
    case UIIndex::UI_WorldMap:
    {
        if (Cursor::drag)
        {
            WorldMap::x = Cursor::x - x;
            WorldMap::y = Cursor::y - y;
            return true;
        }
        else if (WorldMap::mousein())
        {
            x = Cursor::x - WorldMap::x;
            y = Cursor::y - WorldMap::y;
            return true;
        }
    }
    break;
    case UIIndex::UI_MiniMap:
    {
        if (Cursor::drag)
        {
            MiniMap::x = Cursor::x - x;
            MiniMap::y = Cursor::y - y;
            return true;
        }
        else if (MiniMap::mousein())
        {
            x = Cursor::x - MiniMap::x;
            y = Cursor::y - MiniMap::y;
            return true;
        }
    }
    break;
    case UIIndex::UI_UIEquip:
    {
        if (Cursor::drag)
        {
            UIEquip::x = Cursor::x - x;
            UIEquip::y = Cursor::y - y;
            return true;
        }
        else if (UIEquip::mousein())
        {
            x = Cursor::x - UIEquip::x;
            y = Cursor::y - UIEquip::y;
            return true;
        }
    }
    break;
    case UIIndex::UI_UINotice:
    {
        if (Cursor::drag)
        {
            UINotice::x = Cursor::x - x;
            UINotice::y = Cursor::y - y;
            return true;
        }
        else if (UINotice::mousein())
        {
            x = Cursor::x - UINotice::x;
            y = Cursor::y - UINotice::y;
            return true;
        }
    }
    break;
    default:
        break;
    }
    return false;
}

void ui_sort()
{
    for (auto rit = ui_index.rbegin(); rit != ui_index.rend(); ++rit)
    {
        switch (*rit)
        {
        case UIIndex::UI_KeyConfig:
        {
            if (KeyConfig::mousein())
            {
                auto it = --(rit.base());
                ui_index.splice(ui_index.end(), ui_index, it);
                return;
            }
        }
        break;
        case UIIndex::UI_UIItem:
        {
            if (UIItem::mousein())
            {
                auto it = --(rit.base());
                ui_index.splice(ui_index.end(), ui_index, it);
                return;
            }
        }
        break;
        case UIIndex::UI_UISkill:
        {
            if (UISkill::mousein())
            {
                auto it = --(rit.base());
                ui_index.splice(ui_index.end(), ui_index, it);
                return;
            }
        }
        break;
        case UIIndex::UI_UIStat:
        {
            if (UIStat::mousein())
            {
                auto it = --(rit.base());
                ui_index.splice(ui_index.end(), ui_index, it);
                return;
            }
        }
        break;
        case UIIndex::UI_WorldMap:
        {
            if (WorldMap::mousein())
            {
                auto it = --(rit.base());
                ui_index.splice(ui_index.end(), ui_index, it);
                return;
            }
        }
        break;
        case UIIndex::UI_MiniMap:
        {
            if (MiniMap::mousein())
            {
                auto it = --(rit.base());
                ui_index.splice(ui_index.end(), ui_index, it);
                return;
            }
        }
        break;
        case UIIndex::UI_UIEquip:
        {
            if (UIEquip::mousein())
            {
                auto it = --(rit.base());
                ui_index.splice(ui_index.end(), ui_index, it);
                return;
            }
        }
        break;
        case UIIndex::UI_UINotice:
        {
            if (UINotice::mousein())
            {
                auto it = --(rit.base());
                ui_index.splice(ui_index.end(), ui_index, it);
                return;
            }
        }
        break;
        default:
            break;
        }
    }
}

void ui_click()
{
    auto index = ui_index.back();
    switch (index)
    {
    case UIIndex::UI_KeyConfig:
    {
        if (KeyConfig::mousein())
        {
            KeyConfig::click();
            return;
        }
    }
    break;
    case UIIndex::UI_UIItem:
    {
        if (UIItem::mousein())
        {
            UIItem::click();
            return;
        }
    }
    break;
    case UIIndex::UI_UISkill:
    {
        if (UISkill::mousein())
        {
            UISkill::click();
            return;
        }
    }
    break;
    case UIIndex::UI_UIStat:
    {
        if (UIStat::mousein())
        {
            UIStat::click();
            return;
        }
    }
    break;
    case UIIndex::UI_WorldMap:
    {
        if (WorldMap::mousein())
        {
            WorldMap::click();
            return;
        }
    }
    break;
    case UIIndex::UI_UIEquip:
    {
        if (UIEquip::mousein())
        {
            UIEquip::click();
            return;
        }
    }
    break;
    case UIIndex::UI_MiniMap:
    {
        if (MiniMap::mousein())
        {
            MiniMap::click();
            return;
        }
    }
    break;
    case UIIndex::UI_UINotice:
    {
        if (UINotice::mousein())
        {
            UINotice::click();
            return;
        }
    }
    break;
    default:
        break;
    }
    StatusBar::click();
}

void ui_over()
{
    auto mousein = ui_mousein();
    switch (mousein)
    {
    case UIIndex::UI_KeyConfig:
        KeyConfig::over();
        break;
    case UIIndex::UI_UIItem:
        UIItem::over();
        break;
    case UIIndex::UI_UISkill:
        UISkill::over();
        break;
    case UIIndex::UI_UIStat:
        UIStat::over();
        break;
    case UIIndex::UI_WorldMap:
        WorldMap::over();
        break;
    case UIIndex::UI_UIEquip:
        UIEquip::over();
        break;
    case UIIndex::UI_MiniMap:
        MiniMap::over();
        break;
    case UIIndex::UI_UINotice:
        UINotice::over();
        break;
    default:
        StatusBar::over();
        ToolTip::over();
        break;
    }
}

int ui_mousein()
{
    for (auto rit = ui_index.rbegin(); rit != ui_index.rend(); ++rit)
    {
        switch (*rit)
        {
        case UIIndex::UI_KeyConfig:
        {
            if (KeyConfig::mousein())
            {
                return UIIndex::UI_KeyConfig;
            }
        }
        break;
        case UIIndex::UI_UIItem:
        {
            if (UIItem::mousein())
            {
                return UIIndex::UI_UIItem;
            }
        }
        break;
        case UIIndex::UI_UISkill:
        {
            if (UISkill::mousein())
            {
                return UIIndex::UI_UISkill;
            }
        }
        break;
        case UIIndex::UI_UIStat:
        {
            if (UIStat::mousein())
            {
                return UIIndex::UI_UIStat;
            }
        }
        break;
        case UIIndex::UI_WorldMap:
        {
            if (WorldMap::mousein())
            {
                return UIIndex::UI_WorldMap;
            }
        }
        break;
        case UIIndex::UI_MiniMap:
        {
            if (MiniMap::mousein())
            {
                return UIIndex::UI_MiniMap;
            }
        }
        break;
        case UIIndex::UI_UIEquip:
        {
            if (UIEquip::mousein())
            {
                return UIIndex::UI_UIEquip;
            }
        }
        break;
        case UIIndex::UI_UINotice:
        {
            if (UINotice::mousein())
            {
                return UIIndex::UI_UINotice;
            }
        }
        break;
        default:
            break;
        }
    }
    return -1;
}

void ui_keyboard()
{
    for (const auto &[key, value] : Keyboard::ui)
    {
        if (Input::state[key])
        {
            if (!Keyboard::press.contains(key) || Keyboard::press[key] == false)
            {
                switch (value)
                {
                case UIIndex::UI_WorldMap:
                {
                    if (!WorldMap::open)
                    {
                        WorldMap::show();
                    }
                    else
                    {
                        WorldMap::hide();
                    }
                }
                break;
                case UIIndex::UI_UIItem:
                {
                    if (!UIItem::open)
                    {
                        UIItem::show();
                    }
                    else
                    {
                        UIItem::hide();
                    }
                }
                break;
                default:
                    break;
                }
            }
            Keyboard::press[key] = true;
            Cursor::drag = false;
        }
        else
        {
            Keyboard::press[key] = false;
        }
    }
}
