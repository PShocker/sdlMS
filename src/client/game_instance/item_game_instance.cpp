#include "item_game_instance.h"
#include "character_game_instance.h"
#include "character_stat_game_instance.h"
#include "quest_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"
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
      {u"05", u"Cash"}, {u"02", u"Consume"}, {u"04", u"Etc"},
      {u"03", u"Ins"},  {u"50", u"Pet"},     {u"09", u"Special"},
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
  if (type == u"Consume" || type == u"Etc" || type == u"Ins") {
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
    etc.num = num;
    return std::polymorphic<game_item>(std::in_place_type<game_etc_item>, etc);
  } else if (item_type == u"Ins") {
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

void item_game_instance::set_item_num(std::polymorphic<game_item> &itm,
                                      int num) {
  switch (itm->type) {
  case item_enum::consume: {
    auto &consume = static_cast<game_consume_item &>(*itm);
    consume.num = num;
    break;
  }
  case item_enum::etc: {
    auto &etc = static_cast<game_etc_item &>(*itm);
    etc.num = num;
    break;
  }
  default: {
    break;
  }
  }
  quest_game_instance::update_check_item();
  return;
}

void item_game_instance::add_item_num(std::polymorphic<game_item> &item,
                                      int num) {
  switch (item->type) {
  case item_enum::consume: {
    auto &consume = static_cast<game_consume_item &>(*item);
    consume.num += num;
    break;
  }
  case item_enum::etc: {
    auto &etc = static_cast<game_etc_item &>(*item);
    etc.num += num;
    break;
  }
  default: {
    break;
  }
  }
  quest_game_instance::update_check_item();
  return;
}

void item_game_instance::dec_item_num(std::polymorphic<game_item> &item,
                                      int num) {
  switch (item->type) {
  case item_enum::equip: {
    item = std::polymorphic<game_item>(game_equip_item{});
    break;
  }
  case item_enum::consume: {
    auto &consume = static_cast<game_consume_item &>(*item);
    consume.num = consume.num - num;
    if (consume.num == 0) {
      if (!item->id.starts_with(u"0207")) {
        item = std::polymorphic<game_item>(game_consume_item{});
      }
    }
    break;
  }
  case item_enum::etc: {
    auto &etc = static_cast<game_etc_item &>(*item);
    etc.num = etc.num - num;
    if (etc.num == 0) {
      item = std::polymorphic<game_item>(game_etc_item{});
    }
    break;
  }
  case item_enum::install: {
    item = std::polymorphic<game_item>(game_install_item{});
    break;
  }
  case item_enum::cash: {
    item = std::polymorphic<game_item>(game_cash_item{});
    break;
  }
  default: {
    break;
  }
  }
  quest_game_instance::update_check_item();
  return;
}

void item_game_instance::use_morph_item(const std::u16string &id,
                                        game_character &g_character) {
  auto info = item_game_instance::load_item_info(id, 0);
  info = info->find(u"../spec");
  auto morph =
      static_cast<wz::Property<int> *>(info->get_child(u"morph"))->get();
  auto tmp = std::format("{:04d}", morph);
  g_character.morph = {tmp.begin(), tmp.end()};
  g_character.action_index = 0;
  g_character.action_time = 0;
  character_logic_system::run_unsit_chair(g_character);
  if (&g_character == &character_game_instance::self) {
    StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(tmp);
    st.sub_val = 1;
    character_logic_system::ccs.payload.push_back(std::make_unique<StateT>(st));

    st.state = fbs::StateEnum_ITEM_USE;
    st.sub_val = 1;
    character_logic_system::ccs.payload.push_back(std::make_unique<StateT>(st));

    server_character_instance::handle_morph_use(g_character, st);
  }
}

void item_game_instance::unuse_buff_item(const std::u16string &id) {
  for (auto &gib : buff) {
    if (gib.id == id) {
      auto info = item_game_instance::load_item_info(id, 0);
      info = info->find(u"../spec");
      if (info->get_child(u"pad")) {
        auto pad =
            static_cast<wz::Property<int> *>(info->get_child(u"pad"))->get();
        character_stat_game_instance::itm_attack -= pad;
      }
      if (info->get_child(u"mad")) {
        auto mad =
            static_cast<wz::Property<int> *>(info->get_child(u"mad"))->get();
        character_stat_game_instance::itm_magic -= mad;
      }
      if (info->get_child(u"acc")) {
        auto acc =
            static_cast<wz::Property<int> *>(info->get_child(u"acc"))->get();
        character_stat_game_instance::itm_accuracy -= acc;
      }
      if (info->get_child(u"eva")) {
        auto eva =
            static_cast<wz::Property<int> *>(info->get_child(u"eva"))->get();
        character_stat_game_instance::itm_avoid -= eva;
      }
      if (info->get_child(u"speed")) {
        auto speed =
            static_cast<wz::Property<int> *>(info->get_child(u"speed"))->get();
        character_logic_system::self_hspeed_max -= 5;
        character_logic_system::self_hspeed_min += 5;
      }
      if (info->get_child(u"crt")) {
        auto crt =
            static_cast<wz::Property<int> *>(info->get_child(u"crt"))->get();
        character_stat_game_instance::crit_damage -= crt;
      }
      if (info->get_child(u"expBuff")) {
        auto expBuff =
            static_cast<wz::Property<int> *>(info->get_child(u"expBuff"))
                ->get();
        character_stat_game_instance::itm_exp -= expBuff;
      }
      if (info->get_child(u"morph")) {
        auto &sf = character_game_instance::self;
        sf.morph = u"";

        StateT st;
        st.state = fbs::StateEnum_BUFF_ITEM;
        st.val = std::stoi(std::string{sf.morph.begin(), sf.morph.end()});
        st.sub_val = 0;

        auto &ccs = character_logic_system::ccs;
        ccs.payload.push_back(std::make_unique<StateT>(st));
      }
      break;
    }
  }
  std::erase_if(buff, [id](const auto &buff) { return id == buff.id; });
}

bool item_game_instance::use_buff_item(std::polymorphic<game_item> &itm) {
  auto id = itm->id;
  auto info = item_game_instance::load_item_info(id, 0);
  info = info->find(u"../spec");
  if (!info->get_child(u"time")) {
    return false;
  }
  bool r = false;
  unuse_buff_item(itm->id);
  auto time = static_cast<wz::Property<int> *>(info->get_child(u"time"))->get();
  if (info->get_child(u"pad")) {
    auto pad = static_cast<wz::Property<int> *>(info->get_child(u"pad"))->get();
    character_stat_game_instance::itm_attack += pad;
    r = true;
  }
  if (info->get_child(u"mad")) {
    auto mad = static_cast<wz::Property<int> *>(info->get_child(u"mad"))->get();
    character_stat_game_instance::itm_magic -= mad;
    r = true;
  }
  if (info->get_child(u"acc")) {
    auto acc = static_cast<wz::Property<int> *>(info->get_child(u"acc"))->get();
    character_stat_game_instance::itm_accuracy -= acc;
    r = true;
  }
  if (info->get_child(u"eva")) {
    auto eva = static_cast<wz::Property<int> *>(info->get_child(u"eva"))->get();
    character_stat_game_instance::itm_avoid -= eva;
    r = true;
  }
  if (info->get_child(u"speed")) {
    auto speed =
        static_cast<wz::Property<int> *>(info->get_child(u"speed"))->get();
    character_logic_system::self_hspeed_max -= 5;
    character_logic_system::self_hspeed_min += 5;
    r = true;
  }
  if (info->get_child(u"crt")) {
    auto crt = static_cast<wz::Property<int> *>(info->get_child(u"crt"))->get();
    character_stat_game_instance::crit_damage -= crt;
    r = true;
  }
  if (info->get_child(u"expBuff")) {
    auto expBuff =
        static_cast<wz::Property<int> *>(info->get_child(u"expBuff"))->get();
    character_stat_game_instance::itm_exp += expBuff;
  }
  if (info->get_child(u"morph")) {
    use_morph_item(id, character_game_instance::self);
    r = true;
  }
  game_item_buff gib;
  gib.destroy = window::dt_now + time;
  gib.id = id;

  buff.emplace_back(gib);
  return r;
}

bool item_game_instance::use_mob_item(std::polymorphic<game_item> &itm) {
  bool r = false;
  if (character_logic_system::self_fh == 0) {
    return false;
  }
  auto info = item_game_instance::load_item_info(itm->id, 0);
  info = info->find(u"../mob");
  if (info) {
    auto &g_character = character_game_instance::self;
    for (auto [k, v] : *info->get_children()) {
      auto mob_id =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"id"))->get();
      auto mob_num = 1;
      ClientCreateMobT ccm;
      ccm.map_id = scene_system_instance::map_id;
      for (int i = 0; i < mob_num; i++) {
        MobT mt;
        mt.mob_id = mob_id;
        LifeStateT ls;
        ls.x = g_character.pos.x;
        ls.y = g_character.pos.y;
        ls.page = g_character.page;
        ls.fh = character_logic_system::self_fh;
        ls.action_animate = true;
        mt.state = std::make_unique<LifeStateT>(ls);
        ccm.mobs.push_back(std::make_unique<MobT>(mt));
      }
      client_request::send_to_host(ccm);
      r = true;
    }
  }
  return r;
}

bool item_game_instance::use_consume_item(const std::u16string &id) {
  bool r = false;
  auto info = item_game_instance::load_item_info(id, 0);
  info = info->find(u"../spec");
  if (info == nullptr) {
    return false;
  }
  if (info->get_child(u"hp")) {
    auto hp = static_cast<wz::Property<int> *>(info->get_child(u"hp"))->get();
    character_stat_game_instance::hp_point += hp;
    character_stat_game_instance::hp_point =
        std::min(character_stat_game_instance::hp_point,
                 character_stat_game_instance::hp_point_max);
    r = true;
  }
  if (info->get_child(u"hpR")) {
    auto hpR = static_cast<wz::Property<int> *>(info->get_child(u"hpR"))->get();
    character_stat_game_instance::hp_point +=
        hpR * character_stat_game_instance::hp_point_max;
    character_stat_game_instance::hp_point =
        std::min(character_stat_game_instance::hp_point,
                 character_stat_game_instance::hp_point_max);
    r = true;
  }
  if (info->get_child(u"mp")) {
    auto mp = static_cast<wz::Property<int> *>(info->get_child(u"mp"))->get();
    character_stat_game_instance::mp_point += mp;
    character_stat_game_instance::mp_point =
        std::min(character_stat_game_instance::mp_point,
                 character_stat_game_instance::mp_point_max);
    r = true;
  }
  if (info->get_child(u"mpR")) {
    auto mpR = static_cast<wz::Property<int> *>(info->get_child(u"mpR"))->get();
    character_stat_game_instance::mp_point +=
        mpR * character_stat_game_instance::mp_point_max;
    character_stat_game_instance::mp_point =
        std::min(character_stat_game_instance::mp_point,
                 character_stat_game_instance::mp_point_max);
    r = true;
  }
  if (info->get_child(u"moveTo")) {
    auto moveTo =
        static_cast<wz::Property<int> *>(info->get_child(u"moveTo"))->get();
    scene_system_instance::enter_prepare(moveTo, u"sp", 0);
    r = true;
  }
  if (info->get_child(u"exp")) {
    auto exp = static_cast<wz::Property<int> *>(info->get_child(u"exp"))->get();
    character_stat_game_instance::exp_point += exp;
    character_stat_game_instance::upgrade();
    r = true;
  }
  return r;
}

bool item_game_instance::use_consume_item(std::polymorphic<game_item> &itm) {
  bool r = false;
  if (use_buff_item(itm)) {
    r = true;
  }
  if (use_consume_item(itm->id)) {
    r = true;
  }
  if (use_mob_item(itm)) {
    r = true;
  }
  return r;
}

void item_game_instance::use_item(std::polymorphic<game_item> &itm) {
  auto item_type = item_game_instance::load_item_type(itm->id);
  auto &g_character = character_game_instance::self;
  auto self_sit_cooldown = character_logic_system::self_sit_cooldown;

  if (item_type == u"Ins") {
    if (itm->id.starts_with(u"0301") && self_sit_cooldown < window::dt_now) {
      if (!g_character.morph.empty()) {
        return;
      }
      auto action_type = character_logic_system::load_action_type(g_character);
      switch (action_type) {
      case character_logic_system::action_enum::stand: {
        character_logic_system::run_sit_chair(g_character, itm->id);
        break;
      }
      case character_logic_system::action_enum::sit: {
        character_logic_system::run_unsit_chair(g_character);
        break;
      }
      default: {
        break;
      }
      }
      return;
    }
  } else if (item_type == u"Consume") {
    if (use_consume_item(itm)) {
      dec_item_num(itm, 1);
    }
  }
  return;
}