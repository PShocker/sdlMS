#pragma once

#include "src/client/game_instance/craft_game_instance.h"
#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <vector>

namespace sdlms {

class CraftUiSystem : public UiComponentHost<CraftUiSystem> {
 private:
  static void RenderBackgrnd();
  static void RenderLevel();
  static void RenderCategory();
  static void RenderRecipes();
  static void RenderDetail();
  static void RenderBottom();
  static void RenderTooltip();
  static void RenderCraftGauge();

  static bool EventUI(SDL_Event* event);
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static void EventCraft(bool all);
  static void EventCountDelta(int delta);
  static bool Run();

  struct PendingCraft {
    uint32_t item_id = 0;
    uint32_t result_count = 0;
    uint32_t exp = 0;
    uint32_t count = 0;
    uint8_t tab = 0;
  };
  static inline std::optional<PendingCraft> pending_craft_;
  static inline uint64_t craft_finish_time_ = 0;

  static SDL_FPoint LoadWh();

  static std::vector<CraftGameInstance::CraftRecipe> LoadRecipes();
  static bool LoadRecipeItem(uint32_t item_id, SDL_Texture*& texture,
                             std::u16string& name, uint32_t& have);
  static bool CanCraft(const CraftGameInstance::CraftRecipe& recipe, uint32_t count);

  static inline std::optional<SDL_FPoint> drag_;
  static inline uint32_t selected_recipe_ = 0;
  static inline uint32_t recipe_scroll_ = 0;
  static inline uint32_t craft_count_ = 1;
  static inline bool additive_checked_ = false;
  static inline int drag_scroll_ = -1;
  static inline std::optional<size_t> hover_recipe_;
  static inline std::optional<uint32_t> hover_ingredient_;

 public:
  static UIComponents& GetComponent();
  static inline SDL_FPoint pos_;

  static inline uint8_t selected_tab_ = 5;

  static bool Crafting();
  static float CraftProgress();

  static void Open();
  static void Close();

  static bool Render();
  static bool RenderInfo();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
