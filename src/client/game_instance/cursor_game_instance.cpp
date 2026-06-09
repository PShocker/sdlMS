#include "cursor_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include <optional>

void cursor_game_instance::server_cursor_drop(const DropT &dt) {
  if (dt.random_id == cursor_hand_drop_id) {
    cursor_hand_drop_id = std::nullopt;
    auto &hand = cursor_hand.value();
    switch (hand.type) {
    case cursor_game_instance::equipment: {
      break;
    }
    case cursor_game_instance::package: {
      if (hand.val == 0) {
        package_game_instance::equips[hand.val2] = std::nullopt;
      } else {
        std::vector<std::optional<game_item>> *r;
        switch (hand.val) {
        case 1: {
          r = &package_game_instance::cosumes;
          break;
        }
        case 2: {
          r = &package_game_instance::etc;
          break;
        }
        case 3: {
          r = &package_game_instance::install;
          break;
        }
        case 4: {
          r = &package_game_instance::cash;
          break;
        }
        default: {
          break;
        }
        }
        r->at(hand.val2) = std::nullopt;
      }
      break;
    }
    default: {
      break;
    }
    }
    cursor_hand = std::nullopt;
  }
}