#include "UI.h"

std::list<UIIndex> ui_index = {UIIndex::UI_UIBuff, UIIndex::UI_StatusBar};

void ui_run()
{
    auto type = Cursor::type;
    Cursor::type = u"0";
    WorldMap::run();
    UIBuff::run();
    StatusBar::run();
    KeyConfig::run();
    UIItem::run();
    UISkill::run();
    UIStat::run();
    // 检测左键是否被按住
    if (Window::mouse_state & SDL_BUTTON_LMASK)
    {
        Cursor::left_mouse_press = true;
        Cursor::type = u"12";
    }
    else
    {
        if (Cursor::left_mouse_press)
        {
            // 播放点击的声音
            Sound::push(Sound(u"UI.img/BtMouseClick"));
            // 触发点击事件
            WorldMap::click();
            StatusBar::click();
            KeyConfig::click();
            UIItem::click();
            UISkill::click();
            UIStat::click();
        }
        Cursor::left_mouse_press = false;
        Cursor::drag = false;
        WorldMap::over();
    }
    Cursor::action(type);
    Cursor::run();
}

void ui_drag()
{
    static float x = 0;
    static float y = 0;
    auto index = ui_index.back();
    if (index == UIIndex::UI_UIBuff || index == UIIndex::UI_StatusBar)
    {
        return;
    }
    switch (index)
    {
    case UIIndex::UI_KeyConfig:
        break;
    case UIIndex::UI_UIItem:
        break;
    case UIIndex::UI_UISkill:
        break;
    case UIIndex::UI_UIStat:
        break;
    case UIIndex::UI_WorldMap:
        break;
    default:
        break;
    }
}
