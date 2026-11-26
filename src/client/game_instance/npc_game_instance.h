#pragma once

#include "src/client/game/game_npc.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <flat_set>
#include <string>
#include <vector>

namespace sdlms {

class NpcGameInstance {
 public:
  static void Load(uint32_t map_id);
  // 城镇地图没有 WZ 摆设时补一台 MapleTV（本数据包 life 层无 TV）。
  static void EnsureMapleTV(uint32_t map_id);
  static wz::Node* LoadLinkNpcNode(const std::u16string& id);

  static SDL_FRect LoadRect(const GameNpc& g_npc);

  static std::vector<std::u16string> LoadAvaliableQuest(
      const std::u16string& id);
  static std::vector<std::u16string> LoadProgressQuest(
      const std::u16string& id);
  static std::vector<std::u16string> LoadProgressCompleteQuest(
      const std::u16string& id);

  static wz::Node* LoadQuestNode(const GameNpc& g_npc);
  static std::optional<SDL_FRect> LoadQuestRect(const GameNpc& g_npc);

  static std::u16string LoadNpcText(const std::u16string& id,
                                    const std::u16string& val);

  static std::u16string LoadNpcScript(const std::u16string& id);

  enum class NpcType {
    kNone,
    kQuest,
    kShop,
    kScript,
    kStorage,
  };
  static NpcType LoadNpcType(const std::u16string& id);

  // NPCs that own an account storage (仓库) declare the trunk fee in Npc.wz as
  // info/trunkPut (deposit) or info/trunkGet (withdraw).
  static bool CheckStorageNpc(const std::u16string& id);

  // 095 CNpcTemplate::bMapleTV：NPC 模板 info/MapleTV 非 0 即 MapleTV
  // 摆设（本数据包 0800007 Free Market Maple TV）。
  static bool LoadIsMapleTV(const std::u16string& id);
  // UI/MapleTV.img/TVmedia 的段数（095 CMapleTVMan::Init 的
  // m_nMapleTVDefaultMediaNum）。
  static int LoadMapleTVMediaNum();
  // UI/MapleTV.img 下 TV 动画节点缓存（TVon / TVmedia/<n>）。
  static wz::Node* LoadMapleTVNode(const std::u16string& sub);

  static inline std::array<std::vector<GameNpc>, 8> data_;

  static inline const std::flat_set<std::u16string> default_action_ = {
      u"stand",
      u"eye",
  };
};

}  // namespace sdlms
