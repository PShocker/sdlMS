#pragma once
#include <cstdint>
#include <string>

enum class game_chat_enum {
  map,
};
struct game_chat {
  uint64_t client_id;
  game_chat_enum type;
  std::u16string owner;
  std::u16string text;

  uint64_t time;
};