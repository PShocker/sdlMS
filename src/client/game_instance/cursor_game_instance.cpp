#include "cursor_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include <optional>

void cursor_game_instance::server_cursor_drop(const DropT &dt) {
  if (!cursor_hand_net.has_value()) {
    return;
  }
  if (dt.random_id == cursor_hand_net->id) {
    cursor_hand_net = std::nullopt;
    auto &hand = cursor_hand.value();
    switch (hand.type) {
    case cursor_game_instance::equipment: {
      break;
    }
    case cursor_game_instance::package: {
      package_game_instance::data[hand.val][hand.sub_val] = std::nullopt;
      break;
    }
    default: {
      break;
    }
    }
    cursor_hand = std::nullopt;
  }
}