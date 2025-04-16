#pragma once

#include <string>
#include <vector>
#include <map>
#include "Components/Components.h"
#include <SDL3/SDL.h>
#include <functional>

struct StatusBar
{
    static void run();
    static void over();
    static void init();
    static void click();
    static void load();
    static void load_backgrnd3();
    static void load_level();
    static void load_job();
    static void load_name();
    static void load_bar();
    static void load_bar_digit(int x, int cur, int max);
    static void load_bar_fade(float x, float percent, int w);
    static void load_hp();
    static void load_mp();
    static void load_exp();
    static void load_vscr();

    static inline SDL_Texture *backgrnd;
    static inline SDL_Texture *backgrnd2;
    static inline SDL_Texture *backgrnd3;
    static inline SDL_Texture *chatTarget;
    static inline SDL_Texture *iconRed;
    static inline SDL_Texture *iconBlue;
    static inline SDL_Texture *iconMemo;
    static inline SDL_Texture *chat;
    static inline SDL_Texture *box;

    static inline SDL_Texture *graduation;
    static inline SDL_Texture *bar;
    static inline SDL_Texture *gray;

    static inline SDL_Texture *LevelNo[10];
    static inline SDL_Texture *number[14];

    static inline SDL_Texture *job;
    static inline SDL_Texture *name;

    static inline bool chatOpen;
    static inline std::vector<SDL_Texture *> chats;

    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtShop;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtChat;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtNPT;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtMenu;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtShort;

    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtClaim;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> EquipKey;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> InvenKey;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> StatKey;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> SkillKey;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> KeySet;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> QuickSlot;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> QuickSlotD;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtChatMax;

    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, SDL_FRect> position_map = {
        {&BtShop, SDL_FRect{570.0, -34.0, 43.0, 34.0}},
        {&BtChat, SDL_FRect{615.0, -34.0, 43.0, 34.0}},
        {&BtNPT, SDL_FRect{660.0, -34.0, 43.0, 34.0}},
        {&BtMenu, SDL_FRect{705.0, -34.0, 43.0, 34.0}},
        {&BtShort, SDL_FRect{750.0, -34.0, 43.0, 34.0}},
        {&BtClaim, SDL_FRect{570.0, -64.0, 20.0, 19.0}},
        {&EquipKey, SDL_FRect{616.0, -64.0, 28.0, 20.0}},
        {&InvenKey, SDL_FRect{646.0, -64.0, 28.0, 20.0}},
        {&StatKey, SDL_FRect{676.0, -64.0, 28.0, 20.0}},
        {&SkillKey, SDL_FRect{706.0, -64.0, 28.0, 20.0}},
        {&KeySet, SDL_FRect{736.0, -64.0, 28.0, 20.0}},
        {&QuickSlot, SDL_FRect{766.0, -64.0, 28.0, 20.0}},
        {&QuickSlotD, SDL_FRect{796.0, -64.0, 28.0, 20.0}},
        {&BtChatMax, SDL_FRect{536.0, -58.0, 12.0, 12.0}},
    };

    static void QuickSlot_func();
    static void QuickSlotD_func();
    static void KeySet_func();
    static void InvenKey_func();
    static void SkillKey_func();
    static void StatKey_func();
    static void EquipKey_func();

    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, std::function<void()>> click_map = {
        {&QuickSlot, QuickSlot_func},
        {&QuickSlotD, QuickSlotD_func},
        {&KeySet, KeySet_func},
        {&InvenKey, InvenKey_func},
        {&SkillKey, SkillKey_func},
        {&StatKey, StatKey_func},
        {&EquipKey, EquipKey_func},
    };

    static inline int alpha = 255;
};
