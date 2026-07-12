#include "item_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "text_game_instance.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <flat_map>
#include <ranges>
#include <string>

bool item_game_instance::check_item(const std::u16string &id) {
  auto r = id.substr(0, 2);
  if (r == u"01") {
    return false;
  }
  return true;
}

std::u16string item_game_instance::load_item_text(const std::u16string &id,
                                                  const std::u16string &val) {
  auto item_type = load_item_type(id);
  auto str_node = wz_resource::string->find(item_type + u".img");
  if (item_type == u"Etc") {
    str_node = str_node->get_child("Etc");
  }
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());
  str_node = str_node->get_child(result)->get_child(val);
  return text_game_instance::load_rstr(str_node);
}

std::u16string item_game_instance::load_item_type(const std::u16string &id) {
  auto str = id;
  if (str.length() < 8) {
    str.insert(0, 8 - str.size(), u'0');
  }
  static const std::flat_map<std::u16string, std::u16string> types = {
      {u"05", u"Cash"},    {u"02", u"Consume"}, {u"04", u"Etc"},
      {u"03", u"Install"}, {u"50", u"Pet"},     {u"09", u"Special"},
  };
  auto r = str.substr(0, 2);
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

std::unique_ptr<game_item>
item_game_instance::load_item(const std::u16string &id, uint32_t num) {
  auto item_type = item_game_instance::load_item_type(id);
  if (item_type == u"Cash") {
    auto cash = std::make_unique<game_cash_item>();
    cash->id = id;
    return cash;
  } else if (item_type == u"Consume") {
    auto consume = std::make_unique<game_consume_item>();
    consume->id = id;
    consume->num = num;
    return consume;
  } else if (item_type == u"Etc") {
    auto etc = std::make_unique<game_etc_item>();
    etc->id = id;
    etc->num = 1;
    return etc;
  } else if (item_type == u"Install") {
    auto install = std::make_unique<game_install_item>();
    install->id = id;
    return install;
  } else if (item_type == u"Pet" || item_type == u"Special") {
    auto cash = std::make_unique<game_cash_item>();
    cash->id = id;
    return cash;
  }
}