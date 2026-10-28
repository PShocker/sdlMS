#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_TOOLTIP_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_TOOLTIP_GAME_INSTANCE_H_
#include "src/client/game/game_tooltip.h"
#include <flat_map>
#include <vector>
namespace sdlms {

class TooltipGameInstance {
public:
  static void Load(uint32_t map_id);
  static inline std::vector<GameTooltip> data_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_TOOLTIP_GAME_INSTANCE_H_
