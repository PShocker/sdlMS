#include "UI.h"
bool left_mouse_press;
bool right_mouse_press;

void ui_run()
{
    // 检测左键是否被按住
    if (Window::mouse_state & SDL_BUTTON_LMASK)
    {
        left_mouse_press = true;
        SDL_SetCursor(Cursor::cursor[u"12"][0]);
    }
    else
    {
        if (left_mouse_press)
        {
            // 播放点击的声音
            Sound::push(Sound(u"UI.img/BtMouseClick"));
        }
        left_mouse_press = false;
        SDL_SetCursor(Cursor::cursor[u"0"][0]);
    }
}
