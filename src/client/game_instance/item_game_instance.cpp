#include "item_game_instance.h"
#include "src/common/wz/wz_resource.h"
#include "text_game_instance.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
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
  if (str == u"00000000") {
    // meso
    return u"Etc";
  }
  static const std::flat_map<std::u16string, std::u16string> types = {
      {u"05", u"Cash"},    {u"02", u"Consume"}, {u"04", u"Etc"},
      {u"03", u"Install"}, {u"50", u"Pet"},     {u"09", u"Special"},
  };
  auto r = str.substr(0, 2);
  return types.at(r);
}

wz::Node *item_game_instance::load_item_info(const std::u16string &id,
                                             uint32_t num) {
  wz::Node *node;
  if (id == u"00000000") {
    node = wz_resource::item->find(u"Special/0900.img/09000000/iconRaw");
    return node;
  }
  auto type = load_item_type(id);
  if (type == u"Consume" || type == u"Etc" || type == u"Install") {
    auto r = id.substr(0, 4) + u".img";
    node = wz_resource::item->find(type + u"/" + r + u"/" + id)
               ->get_child(u"info");
  }
  return node;
}

std::polymorphic<game_item>
item_game_instance::load_item(const std::u16string &id, uint32_t num) {
  auto item_type = item_game_instance::load_item_type(id);

  if (item_type == u"Cash") {
    game_cash_item cash;
    cash.id = id;
    return std::polymorphic<game_item>(std::in_place_type<game_cash_item>,
                                       cash);
  } else if (item_type == u"Consume") {
    game_consume_item consume;
    consume.id = id;
    consume.num = num;
    return std::polymorphic<game_item>(std::in_place_type<game_consume_item>,
                                       consume);
  } else if (item_type == u"Etc") {
    game_etc_item etc;
    etc.id = id;
    etc.num = 1;
    return std::polymorphic<game_item>(std::in_place_type<game_etc_item>, etc);
  } else if (item_type == u"Install") {
    game_install_item install;
    install.id = id;
    return std::polymorphic<game_item>(std::in_place_type<game_install_item>,
                                       install);
  } else if (item_type == u"Pet" || item_type == u"Special") {
    game_cash_item cash;
    cash.id = id;
    return std::polymorphic<game_item>(std::in_place_type<game_cash_item>,
                                       cash);
  }
  // 处理未知类型 - 返回空或抛出异常
  return std::polymorphic<game_item>(); // 返回空对象
}

int item_game_instance::load_slot_max(const std::u16string &id) {
  int r = 200;
  auto info = load_item_info(id, 0);
  if (info->get_child(u"slotMax")) {
    r = static_cast<wz::Property<int> *>(info->get_child(u"slotMax"))->get();
  }
  return r;
}

int item_game_instance::load_item_num(std::polymorphic<game_item> &itm) {
  int num = 0;
  switch (itm->type) {
  case item_enum::consume: {
    auto &consume = static_cast<game_consume_item &>(*itm);
    num = consume.num;
    break;
  }
  case item_enum::etc: {
    auto &etc = static_cast<game_etc_item &>(*itm);
    num = etc.num;
    break;
  }
  default: {
    break;
  }
  }
  return num;
}