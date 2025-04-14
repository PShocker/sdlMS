#pragma once

#include "Components/Components.h"

struct MiniMap
{
    static void init();
    static void run();
    static void show();
    static void hide();
    static void click();
    static void over();
    static bool mousein();

    static void load_canvas();
    static void load_backgrnd();
    static void load_mapremark();

    static inline int width;
    static inline int height;
    static inline int scale;
    static inline SDL_FPoint center;

    static inline SDL_Texture *canvas;
    static inline SDL_Texture *backgrnd;
    static inline SDL_Texture *mapremark;

    static inline SDL_Texture *user;
    static inline SDL_Texture *npc;
    static inline SDL_Texture *portal;

    struct Wrap
    {
        SDL_FRect r;
        SDL_Texture *texture;
        entt::entity ent;
    };
    static inline std::vector<Wrap> points;

    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtMap;
    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtMin2;

    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, SDL_FRect> position_map = {
        {&BtMap, SDL_FRect{-45.0, 5.0, 39.0, 14.0}},
        {&BtMin2, SDL_FRect{-59.0, 6.0, 12.0, 12.0}},
    };

    static void BtMap_func();
    static void BtMin2_func();

    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, std::function<void()>> click_map = {
        {&BtMap, BtMap_func},
        {&BtMin2, BtMin2_func},
    };

    static inline bool minimize = false;
    static inline float x = 0;
    static inline float y = 0;
    static inline int alpha = 255;
};
