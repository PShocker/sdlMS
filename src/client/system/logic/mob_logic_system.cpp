#include "mob_logic_system.h"
#include "character_logic_system.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <optional>
#include <ranges>
#include <string>
#include <utility>

MobLogicSystem::ActionEnum
MobLogicSystem::LoadActionType(const std::u16string &action) {
  const static std::flat_map<std::u16string, ActionEnum> MapName = {
      {u"stand", ActionEnum::kStand}, {u"move", ActionEnum::kMove},
      {u"hit1", ActionEnum::kHit},    {u"die1", ActionEnum::kDie},
      {u"jump", ActionEnum::kJump},   {u"", ActionEnum::kRevive},
  };
  return MapName.at(action);
}

std::optional<SDL_FRect> MobLogicSystem::LoadRect(const GameMob &g_mob) {
  if (g_mob.action.empty()) {
    return std::nullopt;
  }
  auto mob_node = MobGameInstance::LoadLinkMobNode(g_mob.id);
  auto mob_action_node = mob_node->get_child(g_mob.action);
  bool zigzag = mob_action_node->get_child(u"zigzag") == nullptr ? false : true;
  int32_t canvas_count = mob_action_node->children_count();
  if (zigzag) {
    canvas_count -= 1;
  }
  std::string frame_index;
  if (zigzag && g_mob.ani_index >= canvas_count) {
    frame_index =
        std::to_string(canvas_count - 2 - (g_mob.ani_index % canvas_count));
  } else {
    frame_index = std::to_string(g_mob.ani_index);
  }
  auto texture_node = mob_action_node->get_child(frame_index);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL> *>(texture_node)->get_uol();
  }
  if (texture_node->get_child(u"lt") == nullptr) {
    return std::nullopt;
  }
  auto lt = WzResource::LoadFpoint(texture_node->get_child(u"lt"));
  auto rb = WzResource::LoadFpoint(texture_node->get_child(u"rb"));
  SDL_FRect rect{
      .x = lt.x,
      .y = lt.y,
      .w = rb.x - lt.x,
      .h = rb.y - lt.y,
  };
  rect.x += g_mob.pos.x;
  rect.y += g_mob.pos.y;
  if (g_mob.flip == 1) {
    rect.x += 2 * (g_mob.pos.x - rect.x) - rect.w;
  }
  return rect;
}

std::optional<SDL_FPoint> MobLogicSystem::LoadHead(const GameMob &g_mob) {
  if (g_mob.action.empty()) {
    return std::nullopt;
  }
  auto mob_node = MobGameInstance::LoadLinkMobNode(g_mob.id);
  auto mob_action_node = mob_node->get_child(g_mob.action);
  bool zigzag = mob_action_node->get_child(u"zigzag") == nullptr ? false : true;
  int32_t canvas_count = mob_action_node->children_count();
  if (zigzag) {
    canvas_count -= 1;
  }
  std::string frame_index;
  if (zigzag && g_mob.ani_index >= canvas_count) {
    frame_index =
        std::to_string(canvas_count - 2 - (g_mob.ani_index % canvas_count));
  } else {
    frame_index = std::to_string(g_mob.ani_index);
  }
  auto texture_node = mob_action_node->get_child(frame_index);
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL> *>(texture_node)->get_uol();
  }
  if (texture_node->get_child(u"head") == nullptr) {
    return std::nullopt;
  }
  auto head = WzResource::LoadFpoint(texture_node->get_child(u"head"));
  return head;
}

void MobLogicSystem::RunCollision() {
  if (CharacterLogicSystem::self_invincible_cooldown_ >= Window::dt_now_) {
    return;
  }
  auto &self = CharacterGameInstance::self_;
  if (self.action == u"dead") {
    return;
  }
  std::flat_map<float, GameMob> Mobs;
  for (auto &m : MobGameInstance::data_ | std::views::values) {
    auto action_type = LoadActionType(m.mob.action);
    if (action_type == ActionEnum::kRevive || action_type == ActionEnum::kDie) {
      continue;
    }
    if (m.mob.hp <= 0) {
      continue;
    }
    auto m_r = LoadRect(m.mob).value();
    auto c_r = CharacterLogicSystem::LoadRect(self);
    if (SDL_HasRectIntersectionFloat(&m_r, &c_r)) {
      auto dx = self.pos.x - m.mob.pos.x;
      auto dy = self.pos.y - m.mob.pos.y;
      auto dis = dx * dx + dy * dy;
      Mobs[dis] = m.mob;
    }
  }
  if (!Mobs.empty()) {
    auto &mob = Mobs.begin()->second;
    uint64_t hit_num = 1;
    auto &buff = SkillGameInstance::ski_;
    for (auto &buf : buff) {
      if (buf.hit) {
        hit_num = buf.hit(&mob, hit_num);
      }
    }
    CharacterLogicSystem::RunBeingHit(mob.pos.x, hit_num);
    AttackT at{
        .delay = Window::dt_time_,
        .num = hit_num,
        .x = self.pos.x,
        .y = self.pos.y - 30,
        .type = static_cast<uint8_t>(DamageData::T::kV),
    };
    ServerMobInstance::HandleSAttack(0, at);
    ClientMobAttackT cma;
    cma.map_id = SceneSystemInstance::map_id_;
    cma.payload = std::make_unique<AttackT>(std::move(at));
    ClientRequest::send_to_host(cma);
  }
}

bool MobLogicSystem::RunAnimate(GameMob &g_mob) {
  bool r = false;
  if (!g_mob.ani_animate) {
    return r;
  }
  auto delta = Window::delta_time_;
  uint32_t size = 0;
  g_mob.ani_time += delta;
  //   获取动作延迟时间
  uint32_t delay = 100;

  auto node =
      MobGameInstance::LoadLinkMobNode(g_mob.id)->get_child(g_mob.action);
  bool zigzag = node->get_child(u"zigzag") == nullptr ? false : true;
  int32_t canvas_count = node->children_count();
  if (zigzag) {
    canvas_count -= 1;
  }
  std::string frame_index;
  if (zigzag && g_mob.ani_index >= canvas_count) {
    uint32_t period = 2 * (canvas_count - 1);
    uint32_t phase = g_mob.ani_index % period;
    uint32_t frame_i = (phase <= canvas_count - 1) ? phase : (period - phase);
    frame_index = std::to_string(frame_i);
  } else {
    frame_index = std::to_string(g_mob.ani_index);
  }
  if (auto canvas_node = node->get_child(frame_index)) {
    if (auto delay_node = canvas_node->get_child(u"delay")) {
      if (delay_node->type == wz::Type::String) {
        auto delay2 =
            static_cast<wz::Property<std::u16string> *>(delay_node)->get();
        delay = std::stoi(std::string{delay2.begin(), delay2.end()});
      } else {
        delay = static_cast<wz::Property<int32_t> *>(delay_node)->get();
      }
    }
  }

  if (g_mob.ani_time >= delay) {
    g_mob.ani_index += 1;
    int32_t size = !zigzag ? canvas_count : (canvas_count - 1) * 2 + 1;
    r = g_mob.ani_index >= size;
    g_mob.ani_index = g_mob.ani_index % size;
    g_mob.ani_time = 0;
  }
  return r;
}

void MobLogicSystem::RunAlpha(GameMob &g_mob) {
  auto node =
      MobGameInstance::LoadLinkMobNode(g_mob.id)->get_child(g_mob.action);
  bool zigzag = node->get_child(u"zigzag") == nullptr ? false : true;
  int32_t canvas_count = node->children_count();
  if (zigzag) {
    canvas_count -= 1;
  }
  std::string frame_index;
  if (zigzag && g_mob.ani_index >= canvas_count) {
    frame_index =
        std::to_string(canvas_count - 2 - (g_mob.ani_index % canvas_count));
  } else {
    frame_index = std::to_string(g_mob.ani_index);
  }

  if (auto canvas_node = node->get_child(frame_index)) {
    int32_t a0 = 255;
    int32_t a1 = 255;
    if (canvas_node->get_child(u"a0")) {
      a0 = static_cast<wz::Property<int> *>(canvas_node->get_child(u"a0"))
               ->get();
    }
    if (canvas_node->get_child(u"a1")) {
      a1 = static_cast<wz::Property<int> *>(canvas_node->get_child(u"a1"))
               ->get();
    }
    uint32_t delay = 100;
    if (auto delay_node = canvas_node->get_child(u"delay")) {
      if (delay_node->type == wz::Type::String) {
        auto delay2 =
            static_cast<wz::Property<std::u16string> *>(delay_node)->get();
        delay = std::stoi(std::string{delay2.begin(), delay2.end()});
      } else {
        delay = static_cast<wz::Property<int32_t> *>(delay_node)->get();
      }
    }
    float t = (float)g_mob.ani_time / (float)delay;
    g_mob.alpha = a0 + (a1 - a0) * t;
  }
}

void MobLogicSystem::RunMv() {
  for (auto &c : MobGameInstance::data_ | std::views::values) {
    auto &mvs = c.mvs;
    if (mvs.empty()) {
      continue;
    }
    auto mv = mvs[0];
    auto per = Window::delta_time_ / (float)mv.time;
    // 计算当前点在线段上的进度（基于 x 和 y）
    float dx = mv.x2 - mv.x1;
    float dy = mv.y2 - mv.y1;
    float length_sq = dx * dx + dy * dy;

    if (length_sq > 0) {
      // 投影参数 t：当前点在线段上的位置（0=起点，1=终点）
      float t =
          ((c.mob.pos.x - mv.x1) * dx + (c.mob.pos.y - mv.y1) * dy) / length_sq;
      t = std::clamp(t, 0.0f, 1.0f);
      per += t;
    }
    per = std::min(per, 1.0f);

    auto per_x = mv.x1 + (mv.x2 - mv.x1) * per;
    auto per_y = mv.y1 + (mv.y2 - mv.y1) * per;
    c.mob.pos.x = per_x;
    c.mob.pos.y = per_y;
    c.mob.page = mv.page;
    if (per >= 1.0f) {
      mvs.erase(mvs.begin());
    }
  }
}

void MobLogicSystem::RunDebuff(GameMob &g_mob) {
  std::erase_if(g_mob.debuffs, [](const auto &pair) {
    GameDebuff db = pair.second;
    return db.destroy <= Window::dt_time_;
  });
}

void MobLogicSystem::RunStateMachine(GameMob &g_mob) {
  auto action_type = LoadActionType(g_mob.action);
  if (action_type == ActionEnum::kRevive) {
    return;
  }
  if (RunAnimate(g_mob)) {
    if (action_type == ActionEnum::kDie) {
      g_mob.action = u"";
      return;
    }
  }
  RunAlpha(g_mob);
  RunDebuff(g_mob);
}

bool MobLogicSystem::Run() {
  for (auto &m : MobGameInstance::data_ | std::views::values) {
    RunStateMachine(m.mob);
  }
  RunCollision();
  RunMv();
  return true;
}