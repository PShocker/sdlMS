#include "src/client/system/logic/npc_logic_system.h"

#include "src/client/game/game_chatballoon.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/physic/physic.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <random>
#include <ranges>
#include <string>
#include <vector>

namespace sdlms {
namespace {

constexpr uint64_t kNpcWaitMin = 3000;
constexpr uint64_t kNpcWaitMax = 8000;
constexpr float kMoveForce = 1400.0f;

bool IsFrameKey(const std::u16string& key) {
  return !key.empty() &&
         std::ranges::all_of(key, [](char16_t c) {
           return c >= u'0' && c <= u'9';
         });
}

std::vector<std::u16string> LoadFrameKeys(wz::Node* action) {
  std::vector<std::u16string> keys;
  if (action == nullptr || action->get_children() == nullptr) {
    return keys;
  }
  for (const auto& [key, value] : *action->get_children()) {
    if (!value.empty() && value[0] != nullptr && IsFrameKey(key)) {
      keys.push_back(key);
    }
  }
  std::ranges::sort(keys, [](const std::u16string& a,
                             const std::u16string& b) {
    return std::stoul(std::string{a.begin(), a.end()}) <
           std::stoul(std::string{b.begin(), b.end()});
  });
  return keys;
}

uint32_t FrameNumber(const std::u16string& key) {
  return static_cast<uint32_t>(std::stoul(std::string{key.begin(), key.end()}));
}

uint32_t FirstFrameIndex(wz::Node* action) {
  auto keys = LoadFrameKeys(action);
  return keys.empty() ? 0 : FrameNumber(keys.front());
}

uint32_t FrameDelay(wz::Node* action, const std::u16string& key) {
  auto* frame = action == nullptr ? nullptr : action->get_child(key);
  if (frame == nullptr) {
    return 100;
  }
  auto* delay = frame->get_child(u"delay");
  if (delay == nullptr && frame->type == wz::Type::UOL) {
    if (auto* real = static_cast<wz::Property<wz::WzUOL>*>(frame)->get_uol();
        real != nullptr) {
      delay = real->get_child(u"delay");
    }
  }
  if (delay == nullptr) {
    return 100;
  }
  return static_cast<uint32_t>(
      static_cast<wz::Property<int32_t>*>(delay)->get());
}

uint32_t FrameCycleMs(wz::Node* action) {
  uint32_t total = 0;
  for (const auto& key : LoadFrameKeys(action)) {
    total += FrameDelay(action, key);
  }
  return total;
}

std::vector<std::u16string> LoadSpeakKeys(wz::Node* parent) {
  std::vector<std::u16string> keys;
  if (parent == nullptr) {
    return keys;
  }
  auto* speak = parent->get_child(u"speak");
  if (speak == nullptr || speak->get_children() == nullptr) {
    return keys;
  }
  for (const auto& [key, value] : *speak->get_children()) {
    if (!value.empty() && value[0] != nullptr) {
      keys.push_back(key);
    }
  }
  return keys;
}

std::vector<std::u16string> LoadGestureActions(wz::Node* npc_node) {
  std::vector<std::u16string> actions;
  if (npc_node == nullptr || npc_node->get_children() == nullptr) {
    return actions;
  }
  for (const auto& [key, value] : *npc_node->get_children()) {
    if (key == u"info" || key == u"stand" || key == u"move" ||
        key == u"hide" || key == u"hideName" || value.empty() ||
        value[0] == nullptr) {
      continue;
    }
    if (!LoadFrameKeys(value[0]).empty()) {
      actions.push_back(key);
    }
  }
  return actions;
}

std::u16string LoadIdleAction(wz::Node* npc_node) {
  if (npc_node == nullptr) {
    return u"";
  }
  if (auto* stand = npc_node->get_child(u"stand");
      stand != nullptr && !LoadFrameKeys(stand).empty()) {
    return u"stand";
  }
  if (npc_node->get_children() == nullptr) {
    return u"";
  }
  for (const auto& [key, value] : *npc_node->get_children()) {
    if (key == u"info" || key == u"move" || value.empty() ||
        value[0] == nullptr) {
      continue;
    }
    if (!LoadFrameKeys(value[0]).empty()) {
      return key;
    }
  }
  return u"";
}

bool HasMoveAction(wz::Node* npc_node) {
  if (npc_node == nullptr) {
    return false;
  }
  auto* move = npc_node->get_child(u"move");
  return move != nullptr && !LoadFrameKeys(move).empty();
}

int RandomInt(int min, int max) {
  if (max <= min) {
    return min;
  }
  std::uniform_int_distribution<int> dist(min, max);
  return dist(RandomGameInstance::gen_);
}

uint64_t RandomWait() {
  return Window::dt_now_ +
         static_cast<uint64_t>(RandomInt(static_cast<int>(kNpcWaitMin),
                                         static_cast<int>(kNpcWaitMax)));
}

bool SetChat(GameNpc& g_npc, const std::vector<std::u16string>& keys) {
  if (keys.empty()) {
    return false;
  }
  const auto& key = keys[static_cast<size_t>(RandomInt(
      0, static_cast<int>(keys.size()) - 1))];
  auto text = NpcGameInstance::LoadNpcText(g_npc.id, key);
  if (text.empty()) {
    return false;
  }

  GameChatballoon chatballoon;
  chatballoon.text = std::move(text);
  chatballoon.path = u"0";
  chatballoon.pos = {0, 0};
  chatballoon.size = 14;
  chatballoon.color = SDL_Color{156, 91, 97, 255};
  chatballoon.destroy =
      Window::dt_now_ + static_cast<uint64_t>(RandomInt(4000, 6000));
  g_npc.chatballoon = std::move(chatballoon);
  return true;
}

void SetActionChat(GameNpc& g_npc, wz::Node* action) {
  auto keys = LoadSpeakKeys(action);
  if (!keys.empty()) {
    SetChat(g_npc, keys);
  }
}

}  // namespace

bool NpcLogicSystem::RunAnimate(GameNpc& g_npc) {
  auto* npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  auto* action_node =
      npc_node == nullptr ? nullptr : npc_node->get_child(g_npc.action);
  auto keys = LoadFrameKeys(action_node);
  if (keys.empty()) {
    return true;
  }

  auto current_key = std::to_string(g_npc.ani_index);
  auto it = std::ranges::find(
      keys, std::u16string{current_key.begin(), current_key.end()});
  if (it == keys.end()) {
    g_npc.ani_index = FirstFrameIndex(action_node);
    it = keys.begin();
  }

  g_npc.ani_time += Window::delta_time_;
  const uint32_t delay = FrameDelay(action_node, *it);
  if (g_npc.ani_time < delay) {
    return false;
  }

  g_npc.ani_time = 0;
  ++it;
  if (it == keys.end()) {
    g_npc.ani_index = FirstFrameIndex(action_node);
    return true;
  }
  g_npc.ani_index = FrameNumber(*it);
  return false;
}

void NpcLogicSystem::ResetAnimate(GameNpc& g_npc) {
  g_npc.ani_time = 0;
  auto* npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  auto* action_node =
      npc_node == nullptr ? nullptr : npc_node->get_child(g_npc.action);
  g_npc.ani_index = FirstFrameIndex(action_node);
}

void NpcLogicSystem::EnterStand(GameNpc& g_npc) {
  auto* npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  auto idle = LoadIdleAction(npc_node);
  if (!idle.empty() && idle != g_npc.action) {
    g_npc.action = std::move(idle);
    ResetAnimate(g_npc);
  }
  g_npc.state = NpcState::kStand;
  g_npc.duration = RandomWait();
  g_npc.hforce = std::nullopt;
  g_npc.hspeed = 0.0f;
}

void NpcLogicSystem::EnterAction(GameNpc& g_npc,
                                 const std::u16string& action) {
  auto* npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  auto* action_node =
      npc_node == nullptr ? nullptr : npc_node->get_child(action);
  if (action_node == nullptr || LoadFrameKeys(action_node).empty()) {
    EnterStand(g_npc);
    return;
  }

  g_npc.action = action;
  g_npc.state = NpcState::kAction;
  ResetAnimate(g_npc);
  g_npc.hforce = std::nullopt;
  g_npc.hspeed = 0.0f;
  g_npc.duration =
      Window::dt_now_ +
      std::max<uint64_t>(1000, FrameCycleMs(action_node));
  SetActionChat(g_npc, action_node);
}

void NpcLogicSystem::EnterMove(GameNpc& g_npc) {
  auto* npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  auto* move_node = npc_node == nullptr ? nullptr : npc_node->get_child(u"move");
  if (move_node == nullptr || LoadFrameKeys(move_node).empty()) {
    EnterStand(g_npc);
    return;
  }

  const float low = static_cast<float>(std::min(g_npc.rx0, g_npc.rx1));
  const float high = static_cast<float>(std::max(g_npc.rx0, g_npc.rx1));
  if (high - low < 2.0f) {
    EnterStand(g_npc);
    return;
  }

  bool move_right = false;
  if (g_npc.pos.x <= low + 2.0f) {
    move_right = true;
  } else if (g_npc.pos.x >= high - 2.0f) {
    move_right = false;
  } else {
    move_right = RandomInt(0, 1) != 0;
  }

  g_npc.action = u"move";
  g_npc.state = NpcState::kMove;
  ResetAnimate(g_npc);
  g_npc.hforce = move_right ? kMoveForce : -kMoveForce;
  g_npc.hspeed = 0.0f;
  // 与 Mob/Character 一致：flip=true 表示朝右
  g_npc.flip = move_right;
  g_npc.duration = Window::dt_now_ + static_cast<uint64_t>(RandomInt(3000, 5000));
}

bool NpcLogicSystem::RunMove(GameNpc& g_npc) {
  if (!g_npc.hforce.has_value() || !g_npc.hspeed.has_value()) {
    return false;
  }

  const float low = static_cast<float>(std::min(g_npc.rx0, g_npc.rx1));
  const float high = static_cast<float>(std::max(g_npc.rx0, g_npc.rx1));
  auto border = MapInfoGameInstance::LoadMrBorder(SceneSystemInstance::map_id_);
  border.x = low;
  border.w = high;

  float vspeed = 0.0f;
  const float force = *g_npc.hforce;
  const bool walked =
      Physic::Walk(g_npc.pos, Window::delta_time_ / 1000.0f, *g_npc.hspeed,
                   vspeed, *g_npc.hforce, -100.0f, 100.0f, 800.0f, false,
                   g_npc.fh, border, FootholdGameInstance::data_);
  if (!walked) {
    return false;
  }

  // 撞到 rx0/rx1 时掉头，避免贴着边界持续播放 move
  if (force < 0.0f && g_npc.pos.x <= low + 0.01f) {
    g_npc.hforce = kMoveForce;
    g_npc.hspeed = 0.0f;
    g_npc.flip = true;
  } else if (force > 0.0f && g_npc.pos.x >= high - 0.01f) {
    g_npc.hforce = -kMoveForce;
    g_npc.hspeed = 0.0f;
    g_npc.flip = false;
  }
  return true;
}

void NpcLogicSystem::UpdateChat(GameNpc& g_npc) {
  if (NpcGameInstance::LoadQuestNode(g_npc) != nullptr) {
    g_npc.chatballoon = std::nullopt;
    return;
  }
  if (g_npc.chatballoon.has_value() &&
      g_npc.chatballoon->destroy < Window::dt_now_) {
    g_npc.chatballoon = std::nullopt;
  }
}

void NpcLogicSystem::ChooseNext(GameNpc& g_npc) {
  if (g_npc.chatballoon.has_value() &&
      g_npc.chatballoon->destroy >= Window::dt_now_) {
    g_npc.duration = Window::dt_now_ + 500;
    return;
  }

  auto* npc_node = NpcGameInstance::LoadLinkNpcNode(g_npc.id);
  auto gestures = LoadGestureActions(npc_node);
  auto* info = npc_node == nullptr ? nullptr : npc_node->get_child(u"info");
  auto root_speak = LoadSpeakKeys(info);
  const bool can_move =
      HasMoveAction(npc_node) && std::abs(g_npc.rx1 - g_npc.rx0) >= 2;

  const int roll = RandomInt(0, 99);
  if (can_move && (gestures.empty() || roll < 45)) {
    EnterMove(g_npc);
    return;
  }

  if (!gestures.empty()) {
    // 多数时候做动作；偶尔只说话
    if (!root_speak.empty() && RandomInt(0, 99) < 15 &&
        SetChat(g_npc, root_speak)) {
      g_npc.state = NpcState::kStand;
      g_npc.duration = RandomWait();
      return;
    }
    const auto& action = gestures[static_cast<size_t>(
        RandomInt(0, static_cast<int>(gestures.size()) - 1))];
    EnterAction(g_npc, action);
    return;
  }

  if (!root_speak.empty() && SetChat(g_npc, root_speak)) {
    g_npc.state = NpcState::kStand;
    g_npc.duration = RandomWait();
    return;
  }

  EnterStand(g_npc);
}

bool NpcLogicSystem::Run() {
  for (auto& page : NpcGameInstance::data_) {
    for (auto& g_npc : page) {
      UpdateChat(g_npc);

      switch (g_npc.state) {
        case NpcState::kMove: {
          if (!RunMove(g_npc)) {
            EnterStand(g_npc);
            break;
          }
          RunAnimate(g_npc);
          if (Window::dt_now_ >= g_npc.duration) {
            EnterStand(g_npc);
          }
          break;
        }
        case NpcState::kAction: {
          if (RunAnimate(g_npc) || Window::dt_now_ >= g_npc.duration) {
            EnterStand(g_npc);
          }
          break;
        }
        case NpcState::kStand:
        default: {
          RunAnimate(g_npc);
          if (Window::dt_now_ >= g_npc.duration) {
            ChooseNext(g_npc);
          }
          break;
        }
      }
    }
  }
  return true;
}

}  // namespace sdlms
