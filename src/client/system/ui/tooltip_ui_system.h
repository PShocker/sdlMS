#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <string>
class TooltipUiSystem {
private:
  static int LoadEquipWidth(const std::u16string &id);

  static void RenderBackgrnd(float x, float y, float w, float h);
  static void RenderEquipReq(uint32_t req, uint32_t val,
                               const std::u16string &path, float x, float y);
  static void RenderEquipJob(const std::u16string &id, float x, float y);
  static void RenderEquipBottomInc(std::u16string label, std::u16string val,
                                      float x, float &y);

  static void RenderEquipBottom(GameEquipItem &equip, float x, float y);
  static float LoadEquipBottomH(GameEquipItem &equip);

  static void RenderDecoBottom(GameDecoItem &deco, float x, float y);
  static float LoadDecoBottomH(const std::u16string &id);

  static void RenderSkillBottom(const std::u16string &id, uint8_t level,
                                  float x, float y);
  static float LoadSkillBottomH(const std::u16string &id);

public:
  static void RenderDeco(GameDecoItem &deco, float x, float y);
  static void RenderEquip(GameEquipItem &equip, float x, float y);
  static void RenderItem(GameItem &item, float x, float y);
  static void RenderInstall(GameInstallItem &ins, float x, float y);
  static void RenderSkill(const std::u16string &id, uint8_t level, float x,
                           float y);
  static void RenderWorldMapInfo(uint32_t id, float x, float y);
  static void RenderStr(const std::u16string &str, float x, float y);
  static bool Event(SDL_Event *Event);
};