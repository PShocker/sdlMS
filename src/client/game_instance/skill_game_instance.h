#pragma once
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_sound.h"
#include "wz/Node.h"
#include <string>
#include <vector>
class skill_game_instance {
public:
  static wz::Node *load_skill_node(const std::u16string &id);
};