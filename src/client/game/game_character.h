#pragma once

#include "SDL3/SDL_rect.h"
#include "game_chatballoon.h"
#include "game_equip.h"
#include "game_nametag.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
class game_character {
public:
  std::u8string name;

  uint8_t action_type;
  uint8_t action_index = 0;
  uint8_t action_time = 0;

  std::u16string body;
  std::u16string head;

  std::optional<game_equip> weapon;
  std::optional<game_equip> cap;
  std::optional<game_equip> cape;
  std::optional<game_equip> coat;
  std::optional<game_equip> glove;
  std::optional<game_equip> pant;

  std::optional<game_equip> ring0;
  std::optional<game_equip> ring1;
  std::optional<game_equip> ring2;
  std::optional<game_equip> ring3;

  std::optional<game_equip> shield;
  std::optional<game_equip> longcoat;
  std::optional<game_equip> shoes;

  std::optional<game_equip> accessory;

  std::vector<game_nametag> nametags;

  std::optional<game_chatballoon> chatballoon;

  SDL_FPoint pos;
};