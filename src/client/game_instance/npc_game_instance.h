#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_npc.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_set>
#include <string>
#include <vector>
class NpcGameInstance {
public:
  static void load(uint32_t map_id);
  static wz::Node *load_link_npc_node(const std::u16string &id);

  static SDL_FRect load_rect(const GameNpc &g_npc);

  static std::vector<std::u16string>
  load_avaliable_quest(const std::u16string &id);
  static std::vector<std::u16string>
  load_progress_quest(const std::u16string &id);
  static std::vector<std::u16string>
  load_progress_complete_quest(const std::u16string &id);

  static wz::Node *load_quest_node(const GameNpc &g_npc);
  static std::optional<SDL_FRect> load_quest_rect(const GameNpc &g_npc);

  static std::u16string load_npc_text(const std::u16string &id,
                                      const std::u16string &val);

  static std::u16string load_npc_script(const std::u16string &id);

  enum class NpcType {
    kNone,
    kQuest,
    kShop,
    kScript,
  };
  static NpcType load_npc_type(const std::u16string &id);

  static inline std::array<std::vector<GameNpc>, 8> data;

  static inline const std::flat_set<std::u16string> default_action = {
      u"stand",
      u"eye",
  };
};