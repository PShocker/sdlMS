#include "npc_game_instance.h"
#include "foothold_game_instance.h"
#include "quest_game_instance.h"
#include "shop_game_instance.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "text_game_instance.h"
#include "wz/Node.h"
#include "wz/Property.h"
#include <array>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

std::u16string npc_game_instance::load_npc_text(const std::u16string &id,
                                                const std::u16string &val) {
  std::u16string r = u"";
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });

  std::u16string result(view.begin(), view.end());
  auto str_node = wz_resource::string->find(u"Npc.img/" + result);

  if (auto node = str_node->get_child(val)) {
    r = text_game_instance::load_rstr(node);
  }
  return r;
}

wz::Node *npc_game_instance::load_link_npc_node(const std::u16string &id) {
  auto npc_node = wz_resource::npc->find(id + u".img");
  while (npc_node->find(u"info/link")) {
    auto link_node = npc_node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string> *>(link_node)->get();
    npc_node = wz_resource::npc->find(link + u".img");
  }
  return npc_node;
}

SDL_FRect npc_game_instance::load_rect(const game_npc &g_npc) {
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  // uol
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL> *>(npc_node)->get_uol();
  }
  auto texture = wz_resource::load_texture(npc_node);
  auto origin = wz_resource::load_fpoint(npc_node->get_child(u"origin"));
  SDL_FRect pos_rect = {
      .x = g_npc.pos.x - origin.x,
      .y = g_npc.pos.y - origin.y,
      .w = static_cast<float>(texture->w),
      .h = static_cast<float>(texture->h),
  };
  if (g_npc.flip == 1) {
    pos_rect.x = g_npc.pos.x;
    pos_rect.x = (pos_rect.x - (texture->w - origin.x));
  }
  return pos_rect;
}

wz::Node *npc_game_instance::load_quest_node(const game_npc &g_npc) {
  wz::Node *node = nullptr;
  auto avaliable = npc_game_instance::load_avaliable_quest(g_npc);
  auto progress = npc_game_instance::load_progress_quest(g_npc);
  if (!avaliable.empty()) {
    node = wz_resource::ui->find(u"QuestIcon.img/0");
  } else if (!progress.empty()) {
    node = wz_resource::ui->find(u"QuestIcon.img/1");
  } else {
    return nullptr;
  }
  // 直接用当前时间控制灯泡时间
  auto now = window::dt_now;
  const int delay = 150;
  auto index = std::to_string((now % (node->children_count() * delay)) / delay);
  node = node->get_child(index);
  return node;
}

std::optional<SDL_FRect>
npc_game_instance::load_quest_rect(const game_npc &g_npc) {
  auto node = load_quest_node(g_npc);
  if (node != nullptr) {
    auto npc_rect = npc_game_instance::load_rect(g_npc);
    auto origin = wz_resource::load_fpoint(node->get_child(u"origin"));
    auto texture = wz_resource::load_texture(node);
    SDL_FRect pos_rect{
        g_npc.pos.x - origin.x + (npc_rect.w / 2) + 5,
        g_npc.pos.y - origin.y - npc_rect.h - 10,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    return pos_rect;
  }
  return std::nullopt;
}

void npc_game_instance::load(uint32_t map_id) {
  data = {};

  auto map_node = wz_resource::load_map_node(map_id);
  auto map_life_node = map_node->get_child(u"life");
  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string> *>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"m") {
      continue;
    }
    game_npc g_npc;

    auto npc_node = val[0];
    g_npc.id =
        static_cast<wz::Property<std::u16string> *>(npc_node->get_child(u"id"))
            ->get();
    if (npc_node->get_child(u"f")) {
      g_npc.flip =
          static_cast<wz::Property<int> *>(npc_node->get_child(u"f"))->get();
    }
    g_npc.fh =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"fh"))->get();
    g_npc.rx0 =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"rx0"))->get();
    g_npc.rx1 =
        static_cast<wz::Property<int> *>(npc_node->get_child(u"rx1"))->get();
    auto x = static_cast<wz::Property<int> *>(npc_node->get_child(u"x"))->get();

    auto &fh = foothold_game_instance::data[g_npc.fh];
    auto y = fh.k.value() * x + fh.intercept.value();
    g_npc.pos = {static_cast<float>(x), static_cast<float>(y)};

    // default action
    g_npc.action = u"stand";
    g_npc.duration = window::dt_now + 1000;

    auto page = foothold_game_instance::data.at(g_npc.fh).page;
    data[page].push_back(g_npc);
  }
}

npc_game_instance::npc_type
npc_game_instance::load_npc_type(const std::u16string &id) {
  if (shop_game_instance::load_npc_shop(id)) {
    return npc_type::shop;
  }
  auto npc_node = wz_resource::npc->find(id + u".img");
  if (npc_node->find(u"info/script")) {
    return npc_type::script;
  }
  if (!quest_game_instance::load_npc_quest(id).empty()) {
    return npc_type::quest;
  }
  return npc_type::none;
}

std::vector<std::u16string>
npc_game_instance::load_avaliable_quest(const game_npc &n) {
  auto quests = quest_game_instance::load_npc_quest(n.id);
  std::vector<game_quest> avaliable_quests;
  for (auto q : quests) {
    auto qs = quest_game_instance::load_avaliable_quest(q.quest_id);
    if (qs.has_value()) {
      avaliable_quests.push_back(qs.value());
    }
  }

  std::ranges::sort(quests, {}, &game_quest::quest_id);
  std::ranges::sort(avaliable_quests, {}, &game_quest::quest_id);

  // 直接在视图上操作，避免创建临时vector
  std::vector<std::u16string> result;
  auto q1 = quests | std::views::transform(&game_quest::quest_id);
  auto q2 = avaliable_quests | std::views::transform(&game_quest::quest_id);
  std::ranges::set_intersection(q1, q2, std::back_inserter(result));

  return result;
}

std::vector<std::u16string>
npc_game_instance::load_progress_quest(const game_npc &n) {
  auto quests = quest_game_instance::load_npc_quest(n.id);
  auto progress_quests = quest_game_instance::load_progress_quest();

  std::ranges::sort(quests, {}, &game_quest::quest_id);
  std::ranges::sort(progress_quests, {}, &game_quest::quest_id);

  // 直接在视图上操作，避免创建临时vector
  std::vector<std::u16string> result;
  auto q1 = quests | std::views::transform(&game_quest::quest_id);
  auto q2 = progress_quests | std::views::transform(&game_quest::quest_id);
  std::ranges::set_intersection(q1, q2, std::back_inserter(result));

  return result;
}