#include "text_game_instance.h"
#include "SDL3/SDL_stdinc.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <cstddef>
#include <cstring>
#include <flat_map>
#include <string>

std::u16string text_game_instance::replace_n(std::u16string str) {
  std::u16string_view pattern = u"\\n";
  std::u16string_view replacement = u"\n";
  size_t pos = 0;
  while ((pos = str.find(pattern, pos)) != std::u16string::npos) {
    str.replace(pos, pattern.size(), replacement);
    pos += replacement.size(); // 跳过替换后的内容
  }
  pattern = u"\\r";
  replacement = u"";
  pos = 0;
  while ((pos = str.find(pattern, pos)) != std::u16string::npos) {
    str.replace(pos, pattern.size(), replacement);
    pos += replacement.size(); // 跳过替换后的内容
  }
  return str;
}

std::u16string text_game_instance::replace_r(std::u16string text) {
  std::u16string result = text;
  size_t pos = 0;

  while (true) {
    // 查找 '#'
    size_t start = result.find(u'#', pos);
    if (start == std::u16string::npos)
      break;

    // 检查是否是 #m 或 #p
    if (start + 2 >= result.length())
      break;

    char type = result[start + 1];
    if (type != u'm' && type != u'p' && type != u'o' && type != u't') {
      pos = start + 1;
      continue;
    }

    // 查找结束的 '#'
    size_t end = result.find(u'#', start + 2);
    if (end == std::u16string::npos)
      break;

    // 提取数字
    std::u16string num_str = result.substr(start + 2, end - start - 2);

    // 检查是否全是数字
    bool is_number = true;
    for (char16_t c : num_str) {
      if (!std::isdigit(c)) {
        is_number = false;
        break;
      }
    }

    if (!is_number || num_str.empty()) {
      pos = start + 1;
      continue;
    }

    // 获取替换文本
    std::u16string replacement;
    switch (type) {
    case u'm': {
      // get map name
      int id = std::stoi(std::string{num_str.begin(), num_str.end()});
      auto map_name = minimap_ui_system::load_map_name(id).map_name;
      replacement = map_name;
      break;
    }
    case u'p': {
      auto npc_name = npc_game_instance::load_npc_text(num_str, u"name");
      replacement = npc_name;
      break;
    }
    case u'o': {
      // mob
      auto mob_name = mob_game_instance::load_mob_name(num_str);
      replacement = mob_name;
      break;
    }
    case u't': {
      // item
      auto item_name = item_game_instance::load_item_text(num_str, u"name");
      replacement = item_name;
      break;
    }
    }

    // 执行替换
    result.replace(start, end - start + 1, replacement);
    pos = start + replacement.length();
  }

  return result;
}

std::u16string text_game_instance::load_rstr(wz::Node *node) {
  static std::flat_map<wz::Node *, std::u16string> cache;
  if (!cache.contains(node)) {
    auto str = static_cast<wz::Property<std::u16string> *>(node)->get();
    str = replace_n(str);
    str = replace_r(str);
    cache[node] = str;
  }
  return cache.at(node);
}

std::u16string text_game_instance::load_u16str(const char *text) {
  size_t utf8_len = std::strlen(text);
  char *utf16_data = SDL_iconv_string("UTF-16LE", "UTF-8", text, utf8_len);
  // 直接构造,SDL已经添加了null终止符
  auto u16_ptr = reinterpret_cast<const char16_t *>(utf16_data);
  std::u16string result(u16_ptr); // 自动查找null终止符
  SDL_free(utf16_data);
  return result;
}

std::string text_game_instance::load_u8str(const std::u16string &str) {
  size_t in_bytes = str.length() * sizeof(char16_t);
  char *utf8_text = SDL_iconv_string("UTF-8", "UTF-16LE",
                                     (const char *)str.c_str(), in_bytes);
  std::string result(utf8_text);
  SDL_free(utf8_text);
  return result;
}

std::string text_game_instance::format_with_commas(uint64_t value) {
  char buf[32];
  auto [ptr, ec] = std::to_chars(buf, buf + 32, value);
  std::string s(buf, ptr);
  for (int i = s.size() - 3; i > 0; i -= 3)
    s.insert(i, ",");
  return s;
}