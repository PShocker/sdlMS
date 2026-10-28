#pragma once

#include "SDL3/SDL_rect.h"
#include "wz/Node.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
namespace sdlms {

class MapInfoGameInstance {
public:
  static wz::Node *Load(uint32_t map_id);
  static SDL_FRect LoadVrBorder(uint32_t map_id);
  static SDL_FRect LoadMrBorder(uint32_t map_id);
  static std::optional<SDL_FRect> LoadSwimRect(uint32_t map_id);
  static std::vector<SDL_FPoint> LoadSeat(uint32_t map_id);
  static uint32_t LoadReturnMap(uint32_t map_id);
  static std::u16string LoadBgm(uint32_t map_id);
};

}  // namespace sdlms
