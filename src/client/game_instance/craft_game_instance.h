#pragma once

#include "src/client/game/game_save.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <array>
#include <cstdint>
#include <vector>


namespace sdlms {

class CraftGameInstance {
 public:
  enum class CraftType : uint8_t {
    kSmithing,
    kWeaponcrafting,
    kTailoring,
    kWoodcrafting,
    kLeatherworking,
    kArcforge,
    kLength,
  };
  struct CraftData {
    CraftType type;
    uint8_t level;
    uint32_t exp;
  };
  static void Load(const CharacterSaveData& cs);
  static inline std::array<CraftData, (uint8_t)CraftType::kLength> datas_;

  struct CraftSkillCheck {
    uint32_t skillID;
    uint8_t reqLevel;
  };

  struct CraftInredient {
    uint32_t item;
    uint8_t count;
  };

  struct CraftResult {
    uint32_t item;
    uint32_t count;
    uint32_t exp;
  };

  struct CraftRecipe {
    uint32_t coolTimeSec;
    uint32_t processTimeMS;
    uint32_t meso;
    CraftSkillCheck conditions;
    std::vector<CraftInredient> ingredients;
    CraftResult result;
  };
  static inline std::vector<CraftRecipe> recipes_;

  static void InitRecipe();
  static std::vector<CraftRecipe> LoadSkillRecipes(uint32_t skillID);

};

}  // namespace sdlms
