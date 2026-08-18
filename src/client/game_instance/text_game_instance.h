#pragma once

#include "wz/Node.h"
#include <string>
class text_game_instance {
private:
  static std::u16string replace_n(std::u16string str);
  static std::u16string replace_r(std::u16string text);

public:
  static std::u16string load_rstr(wz::Node *node);

  static std::u16string load_u16str(const char *text);
  static std::string load_u8str(const std::u16string &str);
  static std::string format_with_commas(uint64_t value);
};