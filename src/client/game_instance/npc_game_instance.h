#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_npc.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_set>
#include <string>
#include <vector>
class npc_game_instance {
public:
  static void load(uint32_t map_id);
  static wz::Node *load_link_npc_node(const std::u16string &id);

  static SDL_FRect load_rect(const game_npc &g_npc);

  static std::vector<std::u16string> load_avaliable_quest(const game_npc &n);
  static std::vector<std::u16string> load_progress_quest(const game_npc &n);
  static std::vector<std::u16string> load_complete_quest(const game_npc &n);

  static wz::Node *load_quest_node(const game_npc &g_npc);
  static std::optional<SDL_FRect> load_quest_rect(const game_npc &g_npc);

  static std::u16string load_npc_text(const std::u16string &id,
                                      const std::u16string &val);

  enum class npc_type {
    none,
    quest,
    shop,
    script,
  };
  static npc_type load_npc_type(const std::u16string &id);

  static inline std::array<std::vector<game_npc>, 8> data;

  static inline const std::flat_set<std::u16string> default_action = {
      u"stand",
      u"eye",
  };
};