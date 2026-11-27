#pragma once

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class MapInfoGameInstance {
 public:
  // 095 CField_Coconut：椰子活动的配置（eventName/时间参数）。
  struct CoconutEventInfo {
    std::u16string event_name;
    std::u16string object_name;
    int time_default = 0;
    int time_expand = 0;
    int time_finish = 0;
    int time_message = 0;
  };

  static wz::Node* Load(uint32_t map_id);
  static SDL_FRect LoadVrBorder(uint32_t map_id);
  static SDL_FRect LoadMrBorder(uint32_t map_id);
  static std::optional<SDL_FRect> LoadSwimRect(uint32_t map_id);
  // 095 CField::m_bSwim / m_bFly：地图是否为游泳/飞行地图。
  static bool IsSwimMap(uint32_t map_id);
  static bool IsFlyingMap(uint32_t map_id);
  // 角色是否位于游泳区域内。
  static bool IsSwimming(uint32_t map_id, SDL_FPoint pos);
  static std::vector<SDL_FPoint> LoadSeat(uint32_t map_id);
  // 地图 area 节点：多个矩形区域（095 客户端不直接使用，供脚本/服务端逻辑）。
  static std::vector<SDL_FRect> LoadAreas(uint32_t map_id);
  static bool IsInArea(uint32_t map_id, float x, float y);
  static std::optional<CoconutEventInfo> LoadCoconutEvent(uint32_t map_id);
  // 095 CField::m_dwOption（即 info/fieldLimit）及其常用限制位。
  static int32_t LoadFieldLimit(uint32_t map_id);
  static bool IsSkillDisabled(uint32_t map_id);      // 0x2 禁止技能
  static bool IsShopDisabled(uint32_t map_id);       // 0x10 商店/商城迁移
  static bool IsMinigameDisabled(uint32_t map_id);   // 0x80 小游戏
  static bool IsPortalScrollDisabled(uint32_t map_id);  // 0x100 传送卷
  static bool IsCashItemDisabled(uint32_t map_id);   // 0x40000 现金道具
  static uint32_t LoadReturnMap(uint32_t map_id);
  // 地图附加参数：持续扣血 / 限时 / 强制返回。
  static int32_t LoadDecHP(uint32_t map_id);
  static uint32_t LoadTimeLimit(uint32_t map_id);
  static uint32_t LoadForcedReturn(uint32_t map_id);
  static double LoadRecovery(uint32_t map_id);
  static std::u16string LoadBgm(uint32_t map_id);
};

}  // namespace sdlms
