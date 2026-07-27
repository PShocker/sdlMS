#pragma once

#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "game_chatballoon.h"
#include "game_face.h"
#include "game_install.h"
#include "game_item.h"
#include "game_nametag.h"
#include "game_tomb.h"
#include "src/client/game/game_effect.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

class game_character {
public:
  uint8_t page;

  std::u16string action;
  uint32_t action_index = 0;
  int32_t action_time = 0;
  bool action_animate = true;

  uint8_t alpha = 255;
  uint8_t flip = 0;

  std::u16string body;
  std::u16string head;
  std::u16string ear;
  std::u16string hair;

  game_face face;

  std::optional<game_equip_item> weapon;
  std::optional<game_equip_item> cap;
  std::optional<game_equip_item> cape;
  std::optional<game_equip_item> coat;
  std::optional<game_equip_item> glove;
  std::optional<game_equip_item> pant;

  std::optional<game_equip_item> ring0;
  std::optional<game_equip_item> ring1;
  std::optional<game_equip_item> ring2;
  std::optional<game_equip_item> ring3;

  std::optional<game_equip_item> shield;
  std::optional<game_equip_item> longcoat;
  std::optional<game_equip_item> shoes;

  std::optional<game_equip_item> accessory;

  std::optional<game_deco_item> weapon_deco;
  std::optional<game_deco_item> cap_deco;
  std::optional<game_deco_item> cape_deco;
  std::optional<game_deco_item> coat_deco;
  std::optional<game_deco_item> glove_deco;
  std::optional<game_deco_item> pant_deco;

  std::optional<game_deco_item> ring0_deco;
  std::optional<game_deco_item> ring1_deco;
  std::optional<game_deco_item> ring2_deco;
  std::optional<game_deco_item> ring3_deco;

  std::optional<game_deco_item> shield_deco;
  std::optional<game_deco_item> longcoat_deco;
  std::optional<game_deco_item> shoes_deco;

  std::optional<game_deco_item> accessory_deco;

  std::vector<game_nametag> nametags;

  std::optional<game_chatballoon> chatballoon;

  std::optional<game_install> install;

  SDL_FPoint pos;

  enum class abnormal_state_type {
    dizz,
  };
  std::flat_set<abnormal_state_type> abnormals;

  std::vector<game_effect> effect;
  std::optional<std::u16string> skill;
  std::optional<game_tomb> tomb;

  // characterinfo
  std::u16string job = u"100";
  int32_t fame = 0;
  int32_t level = 0;

  // hide ear
  std::flat_set<std::u16string> hide_part;
};