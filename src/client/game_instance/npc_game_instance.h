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
  static void Load(uint32_t map_id);
  static wz::Node *LoadLinkNpcNode(const std::u16string &id);

  static SDL_FRect LoadRect(const GameNpc &g_npc);

  static std::vector<std::u16string>
  LoadAvaliableQuest(const std::u16string &id);
  static std::vector<std::u16string>
  LoadProgressQuest(const std::u16string &id);
  static std::vector<std::u16string>
  LoadProgressCompleteQuest(const std::u16string &id);

  static wz::Node *LoadQuestNode(const GameNpc &g_npc);
  static std::optional<SDL_FRect> LoadQuestRect(const GameNpc &g_npc);

  static std::u16string LoadNpcText(const std::u16string &id,
                                      const std::u16string &val);

  static std::u16string LoadNpcScript(const std::u16string &id);

  enum class NpcType {
    kNone,
    kQuest,
    kShop,
    kScript,
  };
  static NpcType LoadNpcType(const std::u16string &id);

  static inline std::array<std::vector<GameNpc>, 8> data;

  static inline const std::flat_set<std::u16string> default_action = {
      u"stand",
      u"eye",
  };
};