#include "UI.h"
bool left_mouse_press;
bool right_mouse_press;

void ui_run()
{
    auto type = Cursor::type;
    Cursor::run();
    WorldMap::run();
    UIBuff::run();
    // 检测左键是否被按住
    if (Window::mouse_state & SDL_BUTTON_LMASK)
    {
        left_mouse_press = true;
        Cursor::type = u"12";
    }
    else
    {
        if (left_mouse_press)
        {
            // 播放点击的声音
            Sound::push(Sound(u"UI.img/BtMouseClick"));
            // 触发点击事件
            WorldMap::click();
        }
        left_mouse_press = false;
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
