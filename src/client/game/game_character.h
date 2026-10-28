#pragma once

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "game_chair.h"
#include "game_chatballoon.h"
#include "game_debuff.h"
#include "game_face.h"
#include "game_item.h"
#include "game_tomb.h"
#include "src/client/game/game_effect.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

class GameCharacter {
public:
  uint8_t page;

  std::u16string action;
  uint32_t action_index = 0;
  int32_t action_time = 0;
  bool action_animate = true;

  SDL_Color color = {255, 255, 255, 255};
  uint8_t flip = 0;
  float scale = 1;

  std::u16string body;
  std::u16string head;
  std::u16string ear;
  std::u16string hair;

  GameFace face;

  std::optional<GameEquipItem> weapon;
  std::optional<GameEquipItem> cap;
  std::optional<GameEquipItem> cape;
  std::optional<GameEquipItem> coat;
  std::optional<GameEquipItem> glove;
  std::optional<GameEquipItem> pant;

  std::optional<GameEquipItem> ring0;
  std::optional<GameEquipItem> ring1;
  std::optional<GameEquipItem> ring2;
  std::optional<GameEquipItem> ring3;

  std::optional<GameEquipItem> shield;
  std::optional<GameEquipItem> longcoat;
  std::optional<GameEquipItem> shoes;

  std::optional<GameEquipItem> accessory;

  std::optional<GameDecoItem> weapon_deco;
  std::optional<GameDecoItem> cap_deco;
  std::optional<GameDecoItem> cape_deco;
  std::optional<GameDecoItem> coat_deco;
  std::optional<GameDecoItem> glove_deco;
  std::optional<GameDecoItem> pant_deco;

  std::optional<GameDecoItem> ring0_deco;
  std::optional<GameDecoItem> ring1_deco;

  std::optional<GameDecoItem> shield_deco;
  std::optional<GameDecoItem> longcoat_deco;
  std::optional<GameDecoItem> shoes_deco;

  std::optional<GameDecoItem> accessory_deco;

  std::optional<GameChatballoon> chatballoon;

  std::optional<GameChair> chair;

  SDL_FPoint pos;

  std::flat_map<GameDebuff::DebuffType, GameDebuff> debuffs;

  std::vector<GameEffect> effect;
  std::optional<std::u16string> skill;
  std::optional<GameTomb> tomb;

  // characterinfo
  std::u16string name;
  std::u16string job = u"100";
  int32_t fame = 0;
  int32_t level = 0;

  // hide ear
  std::flat_set<std::u16string> hide_part;

  int8_t attack_speed = 0;

  std::u16string morph;
};