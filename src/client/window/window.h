#ifndef SDLMS_SRC_CLIENT_WINDOW_WINDOW_H_
#define SDLMS_SRC_CLIENT_WINDOW_WINDOW_H_

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL.h>
#include <cstdint>
namespace sdlms {


class Window {
public:
  static inline SDL_Window *win_;
  static inline SDL_Renderer *renderer_;

  static inline SDL_FPoint mouse_pos_;

  static inline SDL_MouseButtonFlags pre_mouse_state_;
  static inline SDL_MouseButtonFlags mouse_state_;
  static const inline bool *keyboard_state_ = SDL_GetKeyboardState(nullptr);

  static inline uint64_t dt_now_ = 0;
  static inline uint64_t dt_last_ = 0;
  static inline int32_t delta_time_ = 0;

  static inline uint64_t dt_time_ = 0;
  static inline uint64_t dt_time_base_ = 0;
  static inline uint64_t dt_time_start_ = 0;
  static inline uint64_t dt_time_end_ = 0;

  static void Create(const char *title, uint32_t logic_w, uint32_t logic_h,
                     uint32_t window_w, uint32_t window_h);

  static void Tick();

  static void Clear();
  static void Update();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_WINDOW_WINDOW_H_
