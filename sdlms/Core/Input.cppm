module;
#include <SDL3/SDL.h>

#include <unordered_set>
export module core:input;

struct Input
{

    static inline std::unordered_set<SDL_Keycode> pressed_keys;
    static inline std::unordered_set<SDL_Keycode> pressed_keys_momentary;
    static inline std::unordered_set<SDL_Keycode> released_keys_momentary;

    static inline std::unordered_set<uint8_t> pressed_mouse;
    static inline std::unordered_set<uint8_t> pressed_mouse_momentary;
    static inline std::unordered_set<uint8_t> released_mouse_momentary;

    static bool is_key_pressed(SDL_Keycode key);
    static bool is_key_held(SDL_Keycode key);
    static bool is_key_released(SDL_Keycode key);

    static bool is_mouse_button_pressed(uint8_t button);
    static bool is_mouse_button_held(uint8_t button);
    static bool is_mouse_button_released(uint8_t button);

    static void process_input_event(int type, const SDL_Event &event);

    static void update_momentary_keys();
};