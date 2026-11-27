#include "src/client/game_instance/npc_game_instance.h"

#include "src/client/game/game_quest.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/shop_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/physic/physic.h"
#include "src/common/wz/wz_resource.h"

#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <array>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <random>
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
  if (str_node == nullptr) {
    return r;
  }

  if (auto node = str_node->get_child(val)) {
    r = TextGameInstance::LoadRstr(node);
  }
  return r;
}

wz::Node* NpcGameInstance::LoadLinkNpcNode(const std::u16string& id) {
  auto npc_node = WzResource::npc_->find(id + u".img");
  // 095 的 link 链可能引用另一个 link NPC，限制跳转次数并防止空节点。
  for (int depth = 0; npc_node != nullptr && depth < 8; ++depth) {
    auto* link_node = npc_node->find(u"info/link");
    if (link_node == nullptr || link_node->type != wz::Type::String) {
      break;
    }
    auto link =
        static_cast<wz::Property<std::u16string>*>(link_node)->get();
    npc_node = WzResource::npc_->find(link + u".img");
  }
  return npc_node;
}

SDL_FRect NpcGameInstance::LoadRect(const GameNpc& g_npc) {
  // NPC 数据缺失（没有 img / 没有该动作 / 没有该帧）时不能解引用空指针，
  // 返回空矩形，调用方（光标命中、任务图标定位）会自然跳过。
  auto npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  if (npc_node == nullptr) {
    return SDL_FRect{};
  }
  auto action_node = npc_node->get_child(g_npc.action);
  if (action_node == nullptr) {
    return SDL_FRect{};
  }

  auto index = std::to_string(g_npc.ani_index);
  npc_node = action_node->get_child(index);
  if (npc_node == nullptr) {
    return SDL_FRect{};
  }
  // uol
  if (npc_node->type == wz::Type::UOL) {
    npc_node = static_cast<wz::Property<wz::WzUOL>*>(npc_node)->get_uol();
  }
  if (npc_node == nullptr) {
    return SDL_FRect{};
  }
  auto texture = WzResource::LoadTexture(npc_node);
  if (texture == nullptr) {
    return SDL_FRect{};
  }
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
    if (texture == nullptr) {
      return std::nullopt;
    }
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

bool NpcGameInstance::LoadIsMapleTV(const std::u16string& id) {
  auto* npc_node = LoadLinkNpcNode(id);
  if (npc_node == nullptr) {
    return false;
  }
  auto* info = npc_node->get_child(u"info");
  if (info == nullptr) {
    return false;
  }
  auto* tv = info->get_child(u"MapleTV");
  if (tv == nullptr || tv->type != wz::Type::Int) {
    return false;
  }
  return static_cast<wz::Property<int32_t>*>(tv)->get() != 0;
}

int NpcGameInstance::LoadMapleTVMediaNum() {
  auto* media = LoadMapleTVNode(u"TVmedia");
  if (media == nullptr || media->get_children() == nullptr) {
    return 0;
  }
  return media->children_count();
}

wz::Node* NpcGameInstance::LoadMapleTVNode(const std::u16string& sub) {
  static std::flat_map<std::u16string, wz::Node*> cache;
  if (auto it = cache.find(sub); it != cache.end()) {
    return it->second;
  }
  auto* node = WzResource::ui_->find(u"MapleTV.img/" + sub);
  cache.emplace(sub, node);
  return node;
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
    // 095 CNpc::Init 的 bMapleTV 分支：MapleTV 摆设随机取一段默认广告
    //（TVmedia/<rand % 段数>）循环播放，外观由 UI/MapleTV.img 绘制。
    if (LoadIsMapleTV(g_npc.id)) {
      g_npc.maple_tv = true;
      const int media_num = LoadMapleTVMediaNum();
      if (media_num > 0) {
        std::uniform_int_distribution<int> pick(0, media_num - 1);
        g_npc.tv_media = static_cast<uint8_t>(pick(RandomGameInstance::gen_));
      }
      g_npc.tv_ani_index = 0;
      g_npc.tv_ani_time = 0;
      // 广告/留言画面偏移（095 SetFlashAbsLocation：npc.pos + ad 偏移）。
      if (auto* info = npc_template->get_child(u"info");
          info != nullptr) {
        auto read = [&info](const char16_t* key) -> int32_t {
          auto* v = info->get_child(key);
          if (v != nullptr && v->type == wz::Type::Int) {
            return static_cast<wz::Property<int32_t>*>(v)->get();
          }
          return 0;
        };
        g_npc.tv_ad_x = read(u"MapleTVadX");
        g_npc.tv_ad_y = read(u"MapleTVadY");
      }
    }
    // 095 SetActive：首次动作/说话等待 3000~8999ms，避免进场淡入期间说话。
    std::uniform_int_distribution<int> first_wait(3000, 8999);
    g_npc.duration =
        Window::dt_now_ + static_cast<uint64_t>(first_wait(RandomGameInstance::gen_));
    g_npc.hforce = std::nullopt;
    g_npc.hspeed = 0.0f;

    auto page = FootholdGameInstance::data_.at(g_npc.fh).page;
    data_[page].push_back(g_npc);
  }

  // MapleTV（NPC 0800007）和其它 NPC 一样，完全由地图 life 层的 WZ 数据生成。
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
  if (NpcGameInstance::CheckStorageNpc(id)) {
    return NpcType::kStorage;
  }
  if (ShopGameInstance::LoadNpcShop(id)) {
    return NpcType::kShop;
  }
  // 一转教官同时挂着二转任务；初心者点击时优先走一转对话，
  // 已转职的角色仍显示任务列表。
  const auto& job = CharacterGameInstance::self_.job;
  if ((job.empty() || job == u"0" || job == u"000") &&
      (id == u"0000221" || id == u"0000313" || id == u"0000411" ||
       id == u"0000511")) {
    return NpcType::kScript;
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

bool NpcGameInstance::CheckStorageNpc(const std::u16string& id) {
  // 仓库 NPC 在 Npc.wz 里用 info/trunkPut（存入费用）/ info/trunkGet
  // （取出费用）声明，比硬编码 id 列表更可靠。
  auto npc_node = NpcGameInstance::LoadLinkNpcNode(id);
  if (npc_node == nullptr) {
    return false;
  }
  return npc_node->find(u"info/trunkPut") != nullptr ||
         npc_node->find(u"info/trunkGet") != nullptr;
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
    // 095 CNpc::SetQuestList：用 CheckCompleteDemand 判断是否进入可交组，
    // 等级 / 物品 / 怪物任一不满足都归到"进行中"。
    if (!QuestGameInstance::CheckCompleteDemand(p.quest_id).empty()) {
      continue;
    }
    if (p.item_bool && p.mob_bool) {
      auto node = QuestGameInstance::LoadQuestNode(p.quest_id);
      if (node == nullptr) {
        continue;
      }
      auto tmp = std::to_string(p.index);
      auto npc_node = node->find("Check/" + tmp + "/npc");
      if (npc_node == nullptr || npc_node->type != wz::Type::Int) {
        continue;
      }
      auto npc_id = static_cast<wz::Property<int>*>(npc_node)->get();
      auto npc_id2 = std::stoi(std::string{id.begin(), id.end()});
      if (npc_id == npc_id2) {
        result.push_back(p.quest_id);
      }
    }
  }
  return result;
}

}  // namespace sdlms
