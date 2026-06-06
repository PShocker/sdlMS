#include "item_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <flat_map>
#include <ranges>
#include <string>

std::u16string item_game_instance::load_item_name(const std::u16string &id) {
  auto item_type = load_item_type(id);
  auto str_node = wz_resource::string->find(item_type + u".img");
  if (item_type == u"Etc") {
    str_node = str_node->get_child("Etc");
  }
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());

  auto str = str_node->get_child(result)->get_child(u"name");
  return static_cast<wz::Property<std::u16string> *>(str)->get();
}

std::u16string item_game_instance::load_item_desc(const std::u16string &id) {
  auto item_type = load_item_type(id);
  auto str_node = wz_resource::string->find(item_type + u".img");
  if (item_type == u"Etc") {
    str_node = str_node->get_child("Etc");
  }
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());

  str_node = str_node->get_child(result)->get_child(u"desc");
  auto str = static_cast<wz::Property<std::u16string> *>(str_node)->get();
  const std::u16string_view pattern = u"\\n";
  const std::u16string_view replacement = u"\n";
  for (size_t pos = 0;
       (pos = str.find(pattern, pos)) != std::u16string::npos;) {
    str.replace(pos, pattern.size(), replacement);
    pos += replacement.size();
  }
  return str;
}

std::u16string item_game_instance::load_item_type(const std::u16string &id) {
  static const std::flat_map<std::u16string, std::u16string> types = {
      {u"05", u"Cash"},    {u"02", u"Consume"}, {u"04", u"Etc"},
      {u"03", u"Install"}, {u"50", u"Pet"},     {u"09", u"Special"},
  };
  auto r = id.substr(0, 2);
  return types.at(r);
}

wz::Node *item_game_instance::load_item_info(const std::u16string &id) {
  auto type = load_item_type(id);
  wz::Node *node;
  if (type == u"Consume" || type == u"Etc" || type == u"Install") {
    auto r = id.substr(0, 4) + u".img";
    node = wz_resource::item->find(type + u"/" + r + u"/" + id)
               ->get_child(u"info");
  }
  return node;
}