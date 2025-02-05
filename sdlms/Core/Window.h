#pragma once

#include <SDL3/SDL.h>

struct Window
{
    static inline SDL_Window *window;
    static inline SDL_Renderer *renderer;

    static inline float mouse_x;
    static inline float mouse_y;
    static inline SDL_MouseButtonFlags mouse_state;

    static inline const char *window_title;

    static inline unsigned int dt_now = 0;
    static inline unsigned int dt_last = 0;
    static inline int delta_time = 0;
    static inline bool quit;

    static void create_window(const char *title, unsigned int width, unsigned int height);

    static void tick_delta_time();

    static void clear();
    static void update();
};
