#include "keyboard_game_instance.h"
#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"

void keyboard_game_instance::reset() {
  data.clear();

  data[SDL_SCANCODE_UP] = {
      .type = input_type::Action,
      .val = (uint32_t)action_type::Up,
  };
  data[SDL_SCANCODE_DOWN] = {
      .type = input_type::Action,
      .val = (uint32_t)action_type::Down,
  };
  data[SDL_SCANCODE_LEFT] = {
      .type = input_type::Action,
      .val = (uint32_t)action_type::Left,
  };
  data[SDL_SCANCODE_RIGHT] = {
      .type = input_type::Action,
      .val = (uint32_t)action_type::Right,
  };
}

void keyboard_game_instance::load() {
    reset();
}