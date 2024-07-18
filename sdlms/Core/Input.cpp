module;

#include <SDL3/SDL.h>

module core;

bool Input::is_key_held(SDL_Keycode key)
{
    return pressed_keys.contains(key);
}

void Input::process_input_event(int type, const SDL_Event &event)
{
    switch (type)
    {
    case SDL_EVENT_KEY_DOWN:
    {
        pressed_keys.insert(event.key.key);
        pressed_keys_momentary.insert(event.key.key);
    }
    break;

    case SDL_EVENT_KEY_UP:
    {
        pressed_keys.erase(event.key.key);
        released_keys_momentary.insert(event.key.key);
    }
    break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    {
        pressed_mouse.insert(event.button.button);
        pressed_mouse_momentary.insert(event.button.button);
    }
    break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
        pressed_mouse.erase(event.button.button);
        released_mouse_momentary.insert(event.button.button);
    }
    break;
    default:
    {
    }
    break;
    }
}

void Input::update_momentary_keys()
{
    pressed_keys_momentary.clear();
    released_keys_momentary.clear();
    pressed_mouse_momentary.clear();
    released_mouse_momentary.clear();
}