#include "src/server/server_instance/server_mob_instance.h"

#include "src/client/game/game_debuff.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "src/server/server_main.h"
#include "src/server/server_system/server_mob_system.h"

#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <flat_map>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace sdlms {

namespace {

constexpr float kWalkSpeedBase = 125.0f;
constexpr float kFlySpeedBase = 125.0f;

int32_t ReadInt(wz::Node* node, const char16_t* key, int32_t def) {
  if (node == nullptr) {
    return def;
  }
  auto* child = node->get_child(key);
  if (child == nullptr || child->type != wz::Type::Int) {
    return def;
  }
  return static_cast<wz::Property<int32_t>*>(child)->get();
}

SDL_FPoint ReadPoint(wz::Node* node, const char16_t* key) {
  SDL_FPoint p{0, 0};
  if (node == nullptr) {
    return p;
  }
  auto* child = node->get_child(key);
  if (child == nullptr || child->type != wz::Type::Vector2D) {
    return p;
  }
  auto v = static_cast<wz::Property<wz::WzVec2D>*>(child)->get();
  p.x = static_cast<float>(v.x);
  p.y = static_cast<float>(v.y);
  return p;
}

// 单帧的延时（wz 里 delay 可能是 int 也可能是字符串）
uint32_t ReadFrameDelay(wz::Node* node) {
  if (node == nullptr) {
    return 0;
  }
  auto* delay_node = node->get_child(u"delay");
  if (delay_node == nullptr) {
    return 0;
  }
  if (delay_node->type == wz::Type::Int) {
    auto v = static_cast<wz::Property<int32_t>*>(delay_node)->get();
    return v < 0 ? 0 : static_cast<uint32_t>(v);
  }
  if (delay_node->type == wz::Type::String) {
    auto s =
        static_cast<wz::Property<std::u16string>*>(delay_node)->get();
    return static_cast<uint32_t>(std::stoi(std::string{s.begin(), s.end()}));
  }
  return 0;
}

// 动作节点下按帧号排序的延时（"10" 必须排在 "2" 之后）
std::map<int32_t, uint32_t> ReadFrameDelays(wz::Node* action_node) {
  std::map<int32_t, uint32_t> delays;
  if (action_node == nullptr) {
    return delays;
  }
  auto* children = action_node->get_children();
  if (children == nullptr) {
    return delays;
  }
  for (const auto& [key, value] : *children) {
    if (key.empty() || key[0] < u'0' || key[0] > u'9') {
      continue;
    }
    if (value.empty()) {
      continue;
    }
    delays[std::stoi(std::string{key.begin(), key.end()})] =
        ReadFrameDelay(value[0]);
  }
  return delays;
}

}  // namespace

wz::Node* ServerMobInstance::LoadMobNode(const std::u16string& mob_id) {
  auto* node = WzResource::mob_->find(mob_id + u".img");
  while (node != nullptr && node->find(u"info/link") != nullptr) {
    auto* link_node = node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string>*>(link_node)->get();
    node = WzResource::mob_->find(link + u".img");
  }
  return node;
}

int32_t ServerMobInstance::LoadMobInfoInt(const std::u16string& mob_id,
                                          const char16_t* key, int32_t def) {
  auto* node = LoadMobNode(mob_id);
  if (node == nullptr) {
    return def;
  }
  return ReadInt(node->get_child(u"info"), key, def);
}

bool ServerMobInstance::HasMobAction(const std::u16string& mob_id,
                                     const std::u16string& action) {
  if (action.empty()) {
    return false;
  }
  auto* node = LoadMobNode(mob_id);
  return node != nullptr && node->get_child(action) != nullptr;
}

std::u16string ServerMobInstance::LoadFineAction(const std::u16string& mob_id,
                                                 const std::u16string& action) {
  if (HasMobAction(mob_id, action)) {
    return action;
  }
  // v95 CMob::GetFineAction：动作缺失时回退到 stand，再回退到 fly
  if (HasMobAction(mob_id, u"stand")) {
    return u"stand";
  }
  if (HasMobAction(mob_id, u"fly")) {
    return u"fly";
  }
  return action;
}

uint64_t ServerMobInstance::LoadActionDelay(const std::u16string& mob_id,
                                            const std::u16string& action) {
  static std::flat_map<std::u16string, uint64_t> cache;
  std::u16string key = mob_id + u"/" + action;
  if (auto it = cache.find(key); it != cache.end()) {
    return it->second;
  }
  uint64_t total = 0;
  auto* node = LoadMobNode(mob_id);
  auto* action_node = node == nullptr ? nullptr : node->get_child(action);
  if (action_node != nullptr) {
    auto delays = ReadFrameDelays(action_node);
    for (const auto& [index, delay] : delays) {
      total += delay;
    }
    // zigzag 动作在播放时会往返一遍：末尾帧不重复
    bool zigzag = action_node->get_child(u"zigzag") != nullptr;
    if (zigzag && delays.size() >= 2) {
      total = total * 2 - delays.begin()->second - delays.rbegin()->second;
    }
  }
  cache[key] = total;
  return total;
}

// v95 CMob::GetHalfWidth：取动作第二帧画布的 cx（没有第二帧就取第一帧）
float ServerMobInstance::LoadMobHalfWidth(const std::u16string& mob_id) {
  static std::flat_map<std::u16string, float> cache;

  auto [it, inserted] = cache.try_emplace(mob_id, 0.0f);
  if (!inserted) {
    return it->second;
  }

  float half = 30.0f;
  auto* node = LoadMobNode(mob_id);
  if (node != nullptr) {
    for (const char16_t* action : {u"stand", u"move", u"fly"}) {
      auto* action_node = node->get_child(action);
      if (action_node == nullptr) {
        continue;
      }
      auto* children = action_node->get_children();
      if (children == nullptr) {
        continue;
      }
      std::map<int32_t, wz::Node*> frames;
      for (const auto& [key, value] : *children) {
        if (key.empty() || key[0] < u'0' || key[0] > u'9' || value.empty()) {
          continue;
        }
        frames[std::stoi(std::string{key.begin(), key.end()})] = value[0];
      }
      if (frames.empty()) {
        continue;
      }
      wz::Node* frame = frames.size() >= 2 ? std::next(frames.begin())->second
                                           : frames.begin()->second;
      if (frame->type == wz::Type::UOL) {
        frame = static_cast<wz::Property<wz::WzUOL>*>(frame)->get_uol();
      }
      if (frame == nullptr || frame->type != wz::Type::Canvas) {
        continue;
      }
      auto canvas = static_cast<wz::Property<wz::WzCanvas>*>(frame)->get();
      half = static_cast<float>(canvas.width) / 2.0f;
      break;
    }
  }
  it->second = half;
  return half;
}

const std::vector<MobAttack>& ServerMobInstance::LoadMobAttacks(
    const std::u16string& mob_id) {
  static std::flat_map<std::u16string, std::vector<MobAttack>> cache;

  auto [it, inserted] = cache.try_emplace(mob_id);
  if (!inserted) {
    return it->second;
  }

  auto& attacks = it->second;
  auto* mob_node = LoadMobNode(mob_id);
  if (mob_node == nullptr) {
    return attacks;
  }

  for (int32_t i = 1; i <= 8; ++i) {
    std::u16string action = u"attack" + std::u16string(1, u'0' + i);
    auto* attack_node = mob_node->get_child(action);
    if (attack_node == nullptr) {
      continue;
    }
    auto* info = attack_node->get_child(u"info");
    if (info == nullptr) {
      continue;
    }
    MobAttack attack;
    attack.action = action;
    attack.magic = ReadInt(info, u"magic", 0) != 0;
    attack.do_first = ReadInt(info, u"doFirst", 0) != 0;
    attack.inactive = ReadInt(info, u"inactive", 0) != 0;
    attack.pad = ReadInt(info, u"PADamage", 0);
    attack.attack_after = static_cast<uint64_t>(ReadInt(info, u"attackAfter", 0));

    // v95 LoadAttackInfo 按 type 选择 range 结构；这里按实际存在的键判断，
    // 兼容缺失 info/type 的数据
    auto* range = info->get_child(u"range");
    if (range != nullptr) {
      if (range->get_child(u"areaCount") != nullptr) {
        attack.type = MobAttack::RangeType::kArea;
        attack.start = ReadInt(range, u"start", 0);
        attack.area_count = std::min(ReadInt(range, u"areaCount", 0), 32);
        attack.attack_count = ReadInt(range, u"attackCount", 0);
      } else if (range->get_child(u"r") != nullptr) {
        attack.type = MobAttack::RangeType::kTrapezoid;
        attack.range = ReadInt(range, u"r", 0);
        attack.sp = ReadPoint(range, u"sp");
      } else {
        attack.type = MobAttack::RangeType::kRect;
      }
      auto lt = ReadPoint(range, u"lt");
      auto rb = ReadPoint(range, u"rb");
      attack.rect = SDL_FRect{
          .x = lt.x,
          .y = lt.y,
          .w = rb.x - lt.x,
          .h = rb.y - lt.y,
      };
    }
    attacks.push_back(std::move(attack));
  }
  return attacks;
}

void ServerMobInstance::LoadDefaultMob(const std::u16string id,
                                       ServerMob& mob) {
  mob.id = id;
  auto* mob_node = LoadMobNode(mob.id);

  // v95 CMobTemplate::RegisterMob 的移动能力推导：
  // 有 fly -> 4；有 jump 且能 move -> 3；只能 move -> 1；否则 0
  bool has_move = mob_node != nullptr && mob_node->get_child(u"move") != nullptr;
  bool has_jump = mob_node != nullptr && mob_node->get_child(u"jump") != nullptr;
  bool has_fly = mob_node != nullptr && mob_node->get_child(u"fly") != nullptr;
  if (has_fly) {
    mob.move_ability = 4;
  } else if (has_jump && has_move) {
    mob.move_ability = 3;
  } else if (has_move) {
    mob.move_ability = 1;
  } else {
    mob.move_ability = 0;
  }
  mob.action = LoadFineAction(mob.id, mob.move_ability == 4 ? u"fly" : u"stand");
  mob.one_shot = false;
  mob.action_end = 0;
  mob.attack_hit = 0;
  mob.dead = false;
  mob.hspeed = 0;
  mob.vspeed = 0;
  mob.hforce = 0;
  mob.input_x = 0;
  mob.think = Window::dt_now_;
  mob.chase_end = 0;
  mob.attack_cd = Window::dt_now_;

  auto* info_node = mob_node == nullptr ? nullptr : mob_node->get_child(u"info");
  mob.hp = ReadInt(info_node, u"maxHP", 1);
  mob.mp = ReadInt(info_node, u"maxMP", 100);

  // walkSpeed = clamp(100 + speed, 10, 140) / 100 * 125，与 v95 属性表一致
  auto speed = std::clamp(ReadInt(info_node, u"speed", 0) + 100, 10, 140);
  mob.hspeed_max = static_cast<float>(speed) / 100.0f * kWalkSpeedBase;

  auto fly_speed = std::clamp(ReadInt(info_node, u"flySpeed", 100), 10, 200);
  mob.fly_speed = static_cast<float>(fly_speed) / 100.0f * kFlySpeedBase;
}

void ServerMobInstance::LoadMob(ServerSceneState& scene) {
  std::flat_map<uint32_t, ServerMob> data;
  auto map_id = scene.map_id;
  auto map_node = WzResource::LoadMapNode(map_id);
  auto map_life_node = map_node->get_child(u"life");
  auto fhs = FootholdGameInstance::Load(map_id);
  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string>*>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"n") {
      continue;
    }
    ServerMob mob;
    auto mob_node = val[0];
    mob.index = std::stoi(std::string{key.begin(), key.end()});
    mob.id =
        static_cast<wz::Property<std::u16string>*>(mob_node->get_child(u"id"))
            ->get();
    mob.fh = static_cast<wz::Property<int>*>(mob_node->get_child(u"fh"))->get();
    mob.rx0 =
        static_cast<wz::Property<int>*>(mob_node->get_child(u"rx0"))->get();
    mob.rx1 =
        static_cast<wz::Property<int>*>(mob_node->get_child(u"rx1"))->get();

    auto x = static_cast<wz::Property<int>*>(mob_node->get_child(u"x"))->get();
    auto m_fh = fhs.at(mob.fh);
    auto y = m_fh.k.value() * x + m_fh.intercept.value();
    mob.pos = {
        static_cast<float>(x),
        static_cast<float>(y),
    };
    mob.page = m_fh.page;
    LoadDefaultMob(mob.id, mob);
    data[mob.index] = mob;
  }
  scene.mobs = data;
}

void ServerMobInstance::HandleAttack(uint64_t client_id,
                                     fbs::ClientMobAttackT& r) {
  if (!ServerClientInstance::clients_.contains(client_id)) {
    return;
  }
  // 转发
  auto map_id = r.map_id;
  auto clients = ServerSceneInstance::scenes_.at(map_id).clients;
  clients.erase(client_id);
  fbs::ServerMobAttackT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    ServerResponse::SendToClient(c, t);
  }
}

void ServerMobInstance::HandleServerMv(const fbs::ServerMobMvT& m) {
  auto& mvs = MobGameInstance::data_.at(m.mob_index).mvs;
  mvs.push_back(*m.payload);
}

void ServerMobInstance::HandleServerFlip(const fbs::ServerMobFlipT& m) {
  auto& mob = MobGameInstance::data_.at(m.mob_index).mob;
  mob.flip = m.payload->flip;
}

void ServerMobInstance::HandleServerAction(const fbs::ServerMobActionT& m) {
  auto& mob = MobGameInstance::data_.at(m.mob_index).mob;
  const auto& a = m.payload;
  mob.action = std::u16string{a->action.begin(), a->action.end()};
  mob.ani_index = 0;
  mob.ani_time = 0;
  mob.ani_animate = true;

  // 服务器复活时先发空动作：客户端用 stand/fly 显示，并在 1 秒内渐显。
  if (mob.action.empty()) {
    auto node = MobGameInstance::LoadLinkMobNode(mob.id);
    if (node != nullptr && node->get_child(u"stand") != nullptr) {
      mob.action = u"stand";
    } else if (node != nullptr && node->get_child(u"fly") != nullptr) {
      mob.action = u"fly";
    }
    mob.alpha = 0;
    mob.revive_time = Window::dt_now_;
  }

  auto MobType = MobLogicSystem::LoadActionType(mob.action);
  switch (MobType) {
    case MobLogicSystem::ActionEnum::kDie: {
      mob.debuffs = {};
      AudioGameInstance::LoadAudio(u"Mob.img/" + mob.id + u"/Die", 0);
      break;
    }
    default: {
      break;
    }
  }
}

void ServerMobInstance::HandleServerState(const fbs::ServerMobStateT& m) {
  auto& mob = MobGameInstance::data_.at(m.mob_index).mob;
  for (const auto& s : m.payload) {
    switch (s->state) {
      case fbs::StateEnum_HP: {
        mob.hp = s->val;
        break;
      }
      default: {
        break;
      }
    }
  }
}

void ServerMobInstance::HandleServerDie(const fbs::ServerMobDieT& m) {
  auto& mob = MobGameInstance::data_.at(m.mob_index).mob;
  float percent = (float)mob.attack_val / mob.max_hp;
  if (percent) {
    percent = std::min(percent, 1.0f);
    auto info_node = MobGameInstance::LoadMobInfo(mob.id);
    auto max_exp =
        static_cast<wz::Property<int>*>(info_node->get_child(u"exp"))->get();
    auto exp = percent * max_exp;
    GainLogGameInstance::Add(u"", exp, GainEnum::kExperience);
    CharacterStatGameInstance::AddExp(exp);
  }
  if (m.client_id == ServerMain::local_addr_) {
    QuestGameInstance::UpdateCheckMob(mob.id, 1);
  } else {
  }
  mob.attack_val = 0;
}

void ServerMobInstance::HandleServerDrop(const fbs::ServerMobDropT& m) {
  // Drop
  for (const auto& dt : m.drop) {
    ServerDropInstance::HandleServerDt(*dt);
  }
  AudioGameInstance::LoadAudio(u"Game.img/DropItem", 0);
  return;
}

void ServerMobInstance::HandleServerEvent(const fbs::ServerMobEventT& m) {
  if (m.map_id != SceneSystemInstance::map_id_) {
    return;
  }
  for (const auto& ev : m.payload) {
    switch (ev.type) {
      case fbs::MobEventUnion_ServerMobMv: {
        const auto mv = ev.AsServerMobMv();
        HandleServerMv(*mv);
        break;
      }
      case fbs::MobEventUnion_ServerMobFlip: {
        const auto fp = ev.AsServerMobFlip();
        HandleServerFlip(*fp);
        break;
      }
      case fbs::MobEventUnion_ServerMobAction: {
        const auto action = ev.AsServerMobAction();
        HandleServerAction(*action);
        break;
      }
      case fbs::MobEventUnion_ServerMobState: {
        const auto st = ev.AsServerMobState();
        HandleServerState(*st);
        break;
      }
      case fbs::MobEventUnion_ServerMobDie: {
        const auto d = ev.AsServerMobDie();
        HandleServerDie(*d);
        break;
      }
      case fbs::MobEventUnion_ServerMobDrop: {
        const auto d = ev.AsServerMobDrop();
        HandleServerDrop(*d);
        break;
      }
      case fbs::MobEventUnion_ServerMobAttackPlayer: {
        const auto d = ev.AsServerMobAttackPlayer();
        HandleServerAttackPlayer(*d);
        break;
      }
      default: {
        break;
      }
    }
  }
}

void ServerMobInstance::HandleSAttack(uint64_t client_id, fbs::AttackT& at) {
  const auto& attack = at;
  // 伤害数字
  DamageData data = {
      .num = attack.num,
      .type = (DamageData::DamageColor)attack.type,
  };
  GameEffect d = {
      .id = u"",
      .index = 0,
      .time = 0,
      .delay = attack.delay,
      .type = GameEffect::EffectType::kDamage,
      .pos = SDL_FPoint{attack.x, attack.y - 10},
      .z = true,
      .flip = false,
      .data = data,
  };
  EffectGameInstance::data_[7].emplace_back(d);
}

void ServerMobInstance::HandleServerAttack(fbs::ServerMobAttackT& r) {
  HandleSAttack(r.client_id, *r.payload);
}

void ServerMobInstance::HandleServerAttackPlayer(
    const fbs::ServerMobAttackPlayerT& m) {
  auto it = MobGameInstance::data_.find(static_cast<int32_t>(m.mob_index));
  if (it == MobGameInstance::data_.end() || m.payload == nullptr) {
    return;
  }
  const auto& g_mob = it->second.mob;

  bool is_self = m.client_id == ServerMain::local_addr_;
  SDL_FPoint target_pos{};
  if (is_self) {
    // v95 的受击无敌期：无敌期间整次攻击被吞掉
    if (CharacterLogicSystem::self_invincible_cooldown_ >= Window::dt_now_) {
      return;
    }
    target_pos = CharacterGameInstance::self_.pos;
  } else if (CharacterGameInstance::others_.contains(m.client_id)) {
    target_pos = CharacterGameInstance::others_.at(m.client_id).g_character.pos;
  } else {
    return;
  }

  // v95 CalcDamage::PDamage + adjust_base_depense：
  // 攻击力已由服务端给出，这里扣掉玩家防御的平坦减伤与百分比减伤
  auto def = CharacterStatGameInstance::GetDef();
  int64_t damage = static_cast<int64_t>(m.payload->num);
  int64_t flat = static_cast<int64_t>(def * 0.25f);
  int64_t pct = static_cast<int64_t>(std::sqrt(def * 0.25f));
  damage = std::min(damage - flat, damage * (100 - pct) / 100);
  if (damage < 1) {
    damage = 1;
  }

  fbs::AttackT at{
      .delay = Window::dt_time_,
      .num = static_cast<uint64_t>(damage),
      .x = target_pos.x,
      .y = target_pos.y - 30,
      .type = static_cast<uint8_t>(DamageData::DamageColor::kWhite),
  };
  HandleSAttack(0, at);

  if (!is_self) {
    return;
  }

  CharacterLogicSystem::RunBeingHit(g_mob.pos.x,
                                    static_cast<uint64_t>(damage));

  // 把真正的伤害数字广播出去，其他客户端才能看到
  fbs::ClientMobAttackT cma;
  cma.map_id = SceneSystemInstance::map_id_;
  cma.payload = std::make_unique<fbs::AttackT>(std::move(at));
  ClientRequest::SendToHost(cma);
}

void ServerMobInstance::HanleServerMob(const std::unique_ptr<fbs::MobT>& m) {
  auto& data = MobGameInstance::data_;
  GameMob g_mob;
  auto tmp = std::format("{:07d}", m->mob_id);
  g_mob.id = {tmp.begin(), tmp.end()};
  g_mob.index = m->mob_index;
  const auto& state = m->state;
  g_mob.action = {
      state->action.begin(),
      state->action.end(),
  };
  g_mob.page = state->page;
  g_mob.flip = state->flip;
  g_mob.pos.x = state->x;
  g_mob.pos.y = state->y;
  g_mob.hp = m->mob_hp;
  auto info_node = MobGameInstance::LoadMobInfo(g_mob.id);
  g_mob.max_hp =
      static_cast<wz::Property<int>*>(info_node->get_child(u"maxHP"))->get();

  g_mob.ani_index = 0;
  g_mob.ani_time = 0;
  g_mob.ani_animate = true;

  auto action_type = MobLogicSystem::LoadActionType(g_mob.action);
  if (action_type == MobLogicSystem::ActionEnum::kDie) {
    g_mob.action = u"";
  }

  data[g_mob.index] = {g_mob, {}};
  return;
}

void ServerMobInstance::HandleCreateMob(fbs::ClientCreateMobT& r) {
  auto map_id = r.map_id;
  auto& scene = ServerSceneInstance::scenes_[map_id];
  int i = 0;
  for (const auto& mb : r.mobs) {
    ServerMob mob;
    mob.create = true;
    mob.index = scene.mobs.size();
    mob.rx0 = INT32_MIN;
    mob.rx1 = INT32_MAX;
    mob.pos = {
        static_cast<float>(mb->state->x),
        static_cast<float>(mb->state->y),
    };
    mob.page = mb->state->page;
    mob.fh = mb->state->fh;
    auto tmp = std::format("{:07d}", mb->mob_id);
    LoadDefaultMob({tmp.begin(), tmp.end()}, mob);
    scene.mobs[mob.index] = mob;

    r.mobs[i]->mob_index = mob.index;
    r.mobs[i]->mob_hp = mob.hp;
    r.mobs[i]->state->action = {mob.action.begin(), mob.action.end()};
    i++;
  }
  fbs::ServerCreateMobT sct;
  sct.mobs = std::move(r.mobs);
  for (auto client_id : scene.clients) {
    ServerResponse::SendToClient(client_id, sct);
  }
  return;
}

void ServerMobInstance::HandleSCreateMob(fbs::ServerCreateMobT& r) {
  for (const auto& m : r.mobs) {
    HanleServerMob(m);
  }
  return;
}

std::vector<ServerMobInstance::MobDrop> ServerMobInstance::LoadMobDrops(
    const std::u16string& mob_id) {
  using CacheType = std::flat_map<std::u16string, std::vector<MobDrop>>;
  static CacheType cache;

  // 使用 try_emplace 一次完成查找和插入
  auto [it, inserted] = cache.try_emplace(mob_id);
  if (!inserted) {
    return it->second;
  }

  auto node = WzResource::ms_->get_root()->find(u"MobDrop.img/" + mob_id);
  if (node == nullptr) {
    return {};
  }

  auto& drops = it->second;
  auto* children = node->get_children();
  drops.reserve(children ? children->size() : 0);

  if (!children) {
    return drops;
  }

  for (auto [k, v] : *children) {
    if (v.empty())
      continue;

    auto* child = v[0];
    if (!child)
      continue;

    auto* min_quantity_prop =
        static_cast<wz::Property<int>*>(child->get_child(u"min_quantity"));
    auto* max_quantity_prop =
        static_cast<wz::Property<int>*>(child->get_child(u"max_quantity"));
    auto* chance_prop =
        static_cast<wz::Property<int>*>(child->get_child(u"chance"));

    if (!min_quantity_prop || !max_quantity_prop || !chance_prop) {
      continue;
    }

    MobDrop md;
    md.id = k;  // 如果后续不需要原字符串，可改为 std::move(k)
    md.min_quantity = min_quantity_prop->get();
    md.max_quantity = max_quantity_prop->get();
    md.rate = chance_prop->get() / 1000000.0f;
    drops.push_back(std::move(md));
  }

  return drops;
}

std::vector<fbs::DropT> ServerMobInstance::CreateMobDts(
    const std::u16string& mob_id, float x, float y, int page) {
  auto mob_drops = LoadMobDrops(mob_id);
  std::vector<fbs::DropT> dts;
  for (const auto& drop : mob_drops) {
    auto& gen_ = RandomGameInstance::gen_;
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    bool success = dis(gen_) <= drop.rate;
    if (!success) {
      continue;
    }
    fbs::DropT dt;
    dt.x1 = x;
    dt.y1 = y;
    dt.page = page;
    if (ItemGameInstance::CheckItem(drop.id)) {
      auto min_quantity = drop.min_quantity;
      auto max_quantity = drop.max_quantity;
      std::uniform_int_distribution<int> dis(min_quantity, max_quantity);
      int random_num = dis(gen_);
      fbs::ItemT it;
      it.item_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
      it.item_num = random_num;
      dt.drop.Set(it);
    } else {
      fbs::EquipT et;
      et.equip_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
      dt.drop.Set(et);
    }
    dts.push_back(dt);
  }
  return dts;
}

void ServerMobInstance::HandleDebuff(uint64_t client_id,
                                     fbs::ClientMobDebuffT& r) {
  auto map_id = r.map_id;
  auto& scene = ServerSceneInstance::scenes_[map_id];
  auto& Mobs = scene.mobs;
  for (auto& db : r.payload) {
    if (Mobs.contains(db->mob_index)) {
      auto& mb = Mobs.at(db->mob_index);
      ServerDebuff sd;
      GameDebuff debuff;
      debuff.type = (GameDebuff::DebuffType)db->payload->val;
      debuff.delay = db->payload->delay;
      debuff.destroy = db->payload->time;

      sd.client_id = client_id;
      sd.db = debuff;
      mb.debuffs[debuff.type] = sd;
    }
  }
  fbs::ServerMobDebuffT smd;
  smd.payload = std::move(r.payload);
  for (auto c : scene.clients) {
    ServerResponse::SendToClient(c, smd);
  }
  return;
}

void ServerMobInstance::HandleSDebuff(fbs::ServerMobDebuffT& r) {
  auto& Mobs = MobGameInstance::data_;
  for (const auto& db : r.payload) {
    if (db->payload->time <= Window::dt_time_) {
      continue;
    }
    if (Mobs.contains(db->mob_index)) {
      auto& mb = Mobs.at(db->mob_index).mob;
      GameDebuff debuff;
      debuff.type = (GameDebuff::DebuffType)db->payload->val;
      debuff.delay = db->payload->delay;
      debuff.destroy = db->payload->time;
      mb.debuffs[debuff.type] = debuff;
    }
  }
}

}  // namespace sdlms
