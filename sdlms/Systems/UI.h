#pragma once

#include <SDL3/SDL.h>
#include "UI/UI.h"
#include "Core/Window.h"
#include "Components/Sound.h"
#include <list>

enum UIIndex
{
    UI_UIBuff,
    UI_StatusBar,
    UI_KeyConfig,
    UI_UIItem,
    UI_UISkill,
    UI_UIStat,
    UI_WorldMap,
    UI_MiniMap,
};
extern std::list<UIIndex> ui_index;

void ui_run();
bool ui_drag();
void ui_sort();
void ui_click();
void ui_over();
