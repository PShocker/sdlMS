#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_animate.h"
#include "src/client/game/game_character.h"
#include <array>
#include <cstdint>
#include <optional>
#include <vector>
namespace sdlms {


class CharacterChooseUiSystem {
private:
  static bool NewAnimate();
  static bool BackAnimate();

  static SDL_FPoint LoadPos();
  static void RenderBackgrnd();
  static void RenderButton();
  static void RenderEffect();
  static void RenderBanner();
  static void RenderCharacter();
  static void RenderCharacterBoard(uint8_t i);
  static void RenderCharacterEffect(uint8_t i);

  static void EventButtonDeleteCb();

  static void EventButtonSelect();
  static void EventButtonNew();
  static void EventButtonDelete();
  static void EventButtonBack();

  static bool EventButton(SDL_Event *Event);
  static bool EventChooseCharacter(SDL_Event *Event);

public:
  static inline std::optional<uint8_t> choose_;
  static inline std::vector<GameCharacter> characters_;
  static inline GameAnimate board_;
  static inline GameAnimate effect_;

  static bool Render();
  static bool Event(SDL_Event *Event);
  static bool Run();
};

}  // namespace sdlms
