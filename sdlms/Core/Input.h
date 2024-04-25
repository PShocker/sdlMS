#pragma once

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_events.h>

#include <unordered_set>

class Input {
public:
	enum class MouseButton { ButtonLeft = 1, ButtonMiddle = 2, ButtonRight = 3 };

private:
	static std::unordered_set<SDL_Keycode> pressed_keys;
	static std::unordered_set<SDL_Keycode> pressed_keys_momentary;
	static std::unordered_set<SDL_Keycode> released_keys_momentary;

	static std::unordered_set<MouseButton> pressed_mouse;
	static std::unordered_set<MouseButton> pressed_mouse_momentary;
	static std::unordered_set<MouseButton> released_mouse_momentary;

public:
	enum class EventType { KeyPressed, KeyReleased, MousePressed, MouseReleased };

	static bool is_key_pressed(SDL_Keycode key);
	static bool is_key_held(SDL_Keycode key);
	static bool is_key_released(SDL_Keycode key);

	static bool is_mouse_button_pressed(MouseButton button);
	static bool is_mouse_button_held(MouseButton button);
	static bool is_mouse_button_released(MouseButton button);

	static void process_input_event(EventType type, const SDL_Event& event);

	static void update_momentary_keys();
};
