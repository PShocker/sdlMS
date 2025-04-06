#pragma once

#include "Components/Components.h"

struct UIItem
{
    static void run();
    static void show();
    static void hide();
    static void over();
    static bool mousein();
    static void click();
    static void click_tab();
    static void load_tab();

    static inline SDL_Texture *FullBackgrnd;
    static inline std::vector<SDL_Texture *> tabs;
    static inline uint8_t active_tab = 0;
    struct Info
    {
        std::u16string id;
        SDL_Texture *texture;
        int num;
    };
    static inline Info infos[5][96];
    static uint8_t load_info_index(std::u16string info_id);
    static inline std::unordered_map<std::u16string, std::pair<uint8_t, uint8_t>> infos_map;
    static std::pair<int, int> full(std::u16string info_id);
    static void push(std::u16string id, int num);

    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>>
        BtClose;

    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, SDL_FRect> position_map = {
        {&BtClose, SDL_FRect{584.0, 6.0, 12.0, 12.0}},
    };

    static void BtClose_func();
    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, std::function<void()>> click_map = {
        {&BtClose, BtClose_func},
    };

    static inline bool open = false;
    static inline bool keyborard = true;
    static inline float x = 0;
    static inline float y = 0;
    static inline int alpha = 255;
};
