module;

#include <SDL3/SDL.h>

export module commons:point;

// 重载 SDL_Point 的加法运算符
export SDL_FPoint operator+(const SDL_FPoint &m, const SDL_FPoint &n);
// 重载 SDL_Point 的减法运算符
export SDL_FPoint operator-(const SDL_FPoint &m, const SDL_FPoint &n);

