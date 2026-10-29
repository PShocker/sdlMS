#pragma once

#include <wz/Node.h>

#include <string>

namespace sdlms {

class TextGameInstance {
 private:
  static std::u16string ReplaceN(std::u16string str);
  static std::u16string ReplaceR(std::u16string text);

 public:
  static std::u16string LoadRstr(wz::Node* node);

  static std::u16string LoadU16str(const char* text);
  static std::string LoadU8str(const std::u16string& str);
  static std::string FormatWithCommas(uint64_t value);
};

}  // namespace sdlms
