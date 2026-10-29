#include "src/client/game_instance/npc_game_instance.h"

#include "src/client/game/game_quest.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/shop_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <wz/Node.h>
#include <wz/Property.h>

#include <array>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {

std::u16string NpcGameInstance::LoadNpcText(const std::u16string& id,
                                            const std::u16string& val) {
  std::u16string r = u"";
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });

  std::u16string result(view.begin(), view.end());
  auto str_node = WzResource::string_->find(u"Npc.img/" + result);

  if (auto node = str_node->get_child(val)) {
    r = TextGameInstance::LoadRstr(node);
  }
  return r;
}

wz::Node* NpcGameInstance::LoadLinkNpcNode(const std::u16string& id) {
  auto npc_node = WzResource::npc_->find(id + u".img");
  while (npc_node->find(u"info/link")) {
    auto link_node = npc_node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string>*>(link_node)->get();
    npc_node = WzResource::npc_->find(link + u".img");
  }
  return npc_node;
}

SDL_FRect NpcGameInstance::LoadRect(const GameNpc& g_npc) {
  auto npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  // uol
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL>*>(npc_node)->get_uol();
  }
  auto texture = WzResource::LoadTexture(npc_node);
  auto origin = WzResource::LoadFpoint(npc_node->get_child(u"origin"));
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

wz::Node* NpcGameInstance::LoadQuestNode(const GameNpc& g_npc) {
  wz::Node* node = nullptr;
  auto avaliable = NpcGameInstance::LoadAvaliableQuest(g_npc.id);
  auto progress = NpcGameInstance::LoadProgressQuest(g_npc.id);
  auto progress_complete = NpcGameInstance::LoadProgressCompleteQuest(g_npc.id);
  if (!progress_complete.empty()) {
    node = WzResource::ui_->find(u"QuestIcon.img/2");
  } else if (!avaliable.empty()) {
    node = WzResource::ui_->find(u"QuestIcon.img/0");
  } else if (!progress.empty()) {
    node = WzResource::ui_->find(u"QuestIcon.img/1");
  } else {
    return nullptr;
  }
  // 直接用当前时间控制灯泡时间
  auto now = Window::dt_now_;
  const int delay = 150;
  auto index = std::to_string((now % (node->children_count() * delay)) / delay);
  node = node->get_child(index);
  return node;
}

std::optional<SDL_FRect> NpcGameInstance::LoadQuestRect(const GameNpc& g_npc) {
  auto node = LoadQuestNode(g_npc);
  if (node != nullptr) {
    auto npc_rect = NpcGameInstance::LoadRect(g_npc);
    auto origin = WzResource::LoadFpoint(node->get_child(u"origin"));
    auto texture = WzResource::LoadTexture(node);
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

void NpcGameInstance::Load(uint32_t map_id) {
  data_ = {};

  auto map_node = WzResource::LoadMapNode(map_id);
  auto map_life_node = map_node->get_child(u"life");
  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string>*>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"m") {
      continue;
    }
    GameNpc g_npc;

    auto npc_node = val[0];
    g_npc.id =
        static_cast<wz::Property<std::u16string>*>(npc_node->get_child(u"id"))
            ->get();
    if (npc_node->get_child(u"f")) {
      g_npc.flip =
          static_cast<wz::Property<int>*>(npc_node->get_child(u"f"))->get();
    }
    g_npc.fh =
        static_cast<wz::Property<int>*>(npc_node->get_child(u"fh"))->get();
    g_npc.rx0 =
        static_cast<wz::Property<int>*>(npc_node->get_child(u"rx0"))->get();
    g_npc.rx1 =
        static_cast<wz::Property<int>*>(npc_node->get_child(u"rx1"))->get();
    auto x = static_cast<wz::Property<int>*>(npc_node->get_child(u"x"))->get();

    auto& fh = FootholdGameInstance::data_[g_npc.fh];
    auto y = fh.k.value() * x + fh.intercept.value();
    g_npc.pos = {static_cast<float>(x), static_cast<float>(y)};

    // default action: stand，没有 stand 的 NPC 选第一个有帧的动作
    g_npc.action = u"stand";
    auto* npc_template = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
    if (npc_template == nullptr ||
        npc_template->get_child(u"stand") == nullptr ||
        npc_template->get_child(u"stand")->get_child(u"0") == nullptr) {
      if (npc_template != nullptr &&
          npc_template->get_children() != nullptr) {
        for (const auto& [key, value] : *npc_template->get_children()) {
          if (key == u"info" || value.empty() || value[0] == nullptr) {
            continue;
          }
          if (value[0]->get_child(u"0") != nullptr) {
            g_npc.action = key;
            break;
          }
        }
      }
    }
    g_npc.state = NpcState::kStand;
    g_npc.ani_index = 0;
    g_npc.ani_time = 0;
    g_npc.duration = Window::dt_now_ + 1000;
    g_npc.hforce = std::nullopt;
    g_npc.hspeed = 0.0f;

    auto page = FootholdGameInstance::data_.at(g_npc.fh).page;
    data_[page].push_back(g_npc);
  }
}

std::u16string NpcGameInstance::LoadNpcScript(const std::u16string& id) {
  auto npc_node = WzResource::npc_->find(id + u".img");
  if (auto n = npc_node->find(u"info/script/0/script"); n != nullptr) {
    return static_cast<wz::Property<std::u16string>*>(n)->get();
  }
  return u"";
}

NpcGameInstance::NpcType NpcGameInstance::LoadNpcType(
    const std::u16string& id) {
  if (ShopGameInstance::LoadNpcShop(id)) {
    return NpcType::kShop;
  }
  auto npc_node = WzResource::npc_->find(id + u".img");
  if (!QuestGameInstance::LoadNpcQuest(id).empty() ||
      !NpcGameInstance::LoadProgressCompleteQuest(id).empty()) {
    return NpcType::kQuest;
  }
  if (npc_node->find(u"info/script")) {
    return NpcType::kScript;
  }
  return NpcType::kNone;
}

std::vector<std::u16string> NpcGameInstance::LoadAvaliableQuest(
    const std::u16string& id) {
  auto quests = QuestGameInstance::LoadNpcQuest(id);
  std::vector<GameQuest> avaliable_quests;
  for (auto q : quests) {
    auto qs = QuestGameInstance::LoadAvaliableQuest(q.quest_id);
    if (qs.has_value()) {
      avaliable_quests.push_back(qs.value());
    }
  }

  std::ranges::sort(quests, {}, &GameQuest::quest_id);
  std::ranges::sort(avaliable_quests, {}, &GameQuest::quest_id);

  // 直接在视图上操作，避免创建临时vector
  std::vector<std::u16string> result;
  auto q1 = quests | std::views::transform(&GameQuest::quest_id);
  auto q2 = avaliable_quests | std::views::transform(&GameQuest::quest_id);
  std::ranges::set_intersection(q1, q2, std::back_inserter(result));

  return result;
}

std::vector<std::u16string> NpcGameInstance::LoadProgressQuest(
    const std::u16string& id) {
  auto quests = QuestGameInstance::LoadNpcQuest(id);
  auto progress_quests = QuestGameInstance::LoadProgressQuest();

  std::ranges::sort(quests, {}, &GameQuest::quest_id);
  std::ranges::sort(progress_quests, {}, &GameQuest::quest_id);

  // 直接在视图上操作，避免创建临时vector
  std::vector<std::u16string> result;
  auto q1 = quests | std::views::transform(&GameQuest::quest_id);
  auto q2 = progress_quests | std::views::transform(&GameQuest::quest_id);
  std::ranges::set_intersection(q1, q2, std::back_inserter(result));

  return result;
}

std::vector<std::u16string> NpcGameInstance::LoadProgressCompleteQuest(
    const std::u16string& id) {
  std::vector<std::u16string> result;
  auto progress = QuestGameInstance::LoadProgressQuest();
  for (auto& p : progress) {
    if (p.item_bool && p.mob_bool) {
      auto node = QuestGameInstance::LoadQuestNode(p.quest_id);
      auto tmp = std::to_string(p.index);
      node = node->find("Check/" + tmp + "/npc");
      auto npc_id = static_cast<wz::Property<int>*>(node)->get();
      auto npc_id2 = std::stoi(std::string{id.begin(), id.end()});
      if (npc_id == npc_id2) {
        result.push_back(p.quest_id);
      }
    }
  }
  return result;
}

}  // namespace sdlms
