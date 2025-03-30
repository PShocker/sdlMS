#include "UI.h"

void ui_run()
{
    auto type = Cursor::type;
    Cursor::run();
    WorldMap::run();
    UIBuff::run();
    StatusBar::run();
    KeyConfig::run();
    UIItem::run();
    UISkill::run();
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
        }
        Cursor::left_mouse_press = false;
        if (WorldMap::over())
        {
            Cursor::type = u"1";
        }
        else
        {
            Cursor::type = u"0";
        }
    }
    Cursor::action(type);
}
