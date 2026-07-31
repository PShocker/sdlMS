#include "drop_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_drop.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include <format>
#include <optional>

void drop_game_instance::pick_drop(uint64_t client_id,
                                   ServerCharacterDropT &r) {
  auto random_id = r.payload->random_id;
  if (drop_game_instance::data.contains(random_id)) {
    auto &drop = drop_game_instance::data.at(random_id);
    drop.type = game_drop::pick;
    drop.picker = {
        .client_id = r.client_id,
        .pet_id = std::nullopt,
    };
  }
}