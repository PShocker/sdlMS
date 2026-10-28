#include "src/server/server_system/server_mob_system.h"
#include <SDL3/SDL_rect.h>
#include "src/client/game/game_debuff.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include <wz/Property.h>
#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
namespace sdlms {


// 常量定义
namespace {
constexpr float MOVE_FORCE = 1400.0f;
constexpr uint32_t MIN_FRAME_INTERVAL_MS = 33;
} // namespace

bool ServerMobSystem::RunTryJump(ServerMob &mob) {
  auto mob_node = MobGameInstance::LoadLinkMobNode(mob.id);
  if (!mob_node->children.contains(u"jump")) {
    return false;
  }
  const auto &s_fhs = ServerSceneInstance::scenes_.at(map_id_).fhs;
  const auto &s_fh = s_fhs.at(mob.fh).fh;
  int32_t n_fh = -1;
  if (mob.flip) {
    n_fh = s_fh.prev;
  } else {
    n_fh = s_fh.next;
  }
  auto &gen = RandomGameInstance::gen;
  bool jump = false;
  std::bernoulli_distribution dist(0.005);
  jump = dist(gen);
  if (s_fhs.contains(n_fh)) {
    if (!s_fhs.at(n_fh).fh.k.has_value()) {
      // wall
      std::bernoulli_distribution dist(0.4);
      jump = dist(gen);
    }
  }
  if (jump) {
    mob.action = u"jump";
    mob.vspeed = -555;
    mob.fh = 0;
  }
  return jump;
}

int ServerMobSystem::LoadMobHitCd(ServerMob &mob) {
  int cd = 0;
  auto mob_node = MobGameInstance::LoadLinkMobNode(mob.id);
  mob_node = mob_node->get_child(u"hit1");
  for (auto [k, v] : *mob_node->get_children()) {
    if (v[0]->get_child(u"delay")) {
      auto delay =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"delay"))->get();
      cd += delay;
    }
  }
  return cd;
}

void ServerMobSystem::RunNetworkSyncPos(ServerMob &mob,
                                             ServerMob &o_mob) {
  if (o_mob.pos.x != mob.pos.x || o_mob.pos.y != mob.pos.y) {
    fbs::MovementT mv;
    mv.x1 = o_mob.pos.x;
    mv.y1 = o_mob.pos.y;
    mv.x2 = mob.pos.x;
    mv.y2 = mob.pos.y;
    mv.page = mob.page;
    mv.time = std::min((uint32_t)Window::delta_time_, MIN_FRAME_INTERVAL_MS);

    fbs::ServerMobMvT mvt;
    mvt.mob_index = mob.index;
    mvt.payload = std::make_unique<fbs::MovementT>(std::move(mv));

    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(mvt));
    events_.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::RunNetworkSyncFlip(ServerMob &mob,
                                              ServerMob &o_mob) {
  if (o_mob.flip != mob.flip) {
    fbs::FlipT ft;
    ft.flip = mob.flip;

    fbs::ServerMobFlipT smf;
    smf.mob_index = mob.index;
    smf.payload = std::make_unique<fbs::FlipT>(std::move(ft));

    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(smf));
    events_.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::RunNetworkSyncAction(ServerMob &mob,
                                                ServerMob &o_mob) {
  if (o_mob.action != mob.action) {
    fbs::ActionT a;
    a.action = std::string{mob.action.begin(), mob.action.end()};
    a.action_animate = true;
    a.action_index = 0;

    fbs::ServerMobActionT sma;
    sma.mob_index = mob.index;
    sma.payload = std::make_unique<fbs::ActionT>(std::move(a));

    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(sma));
    events_.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::RunNetworkSyncHp(ServerMob &mob,
                                            ServerMob &o_mob) {
  if (o_mob.hp != mob.hp) {
    fbs::StateT st;
    st.state = fbs::StateEnum_HP;
    st.val = mob.hp;

    fbs::ServerMobStateT smb;
    smb.mob_index = mob.index;
    smb.payload.push_back(std::make_unique<fbs::StateT>(std::move(st)));

    fbs::MobEventUnionUnion muu;
    muu.Set(std::move(smb));
    events_.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::RunNetworkSync(ServerMob &mob, ServerMob &o_mob) {
  RunNetworkSyncPos(mob, o_mob);
  RunNetworkSyncFlip(mob, o_mob);
  RunNetworkSyncAction(mob, o_mob);
  RunNetworkSyncHp(mob, o_mob);
}

void ServerMobSystem::RunWalk(ServerMob &mob) {
  SDL_FRect border;
  border.x = mob.rx0;
  border.w = mob.rx1;

  Physic::Walk(mob.pos, delta_time_ / 1000.0f, mob.hspeed, mob.vspeed,
               mob.hforce, -mob.hspeed_max, mob.hspeed_max, 0, false, mob.fh,
               border, g_fhs_);
}

void ServerMobSystem::RunDuration(ServerMob &mob) {
  if (mob.duration > Window::dt_now_) {
    return;
  }

  auto action_type = MobLogicSystem::LoadActionType(mob.action);
  if (action_type == MobLogicSystem::ActionEnum::kJump ||
      action_type == MobLogicSystem::ActionEnum::kHit ||
      action_type == MobLogicSystem::ActionEnum::kDie) {
    return;
  }

  static const std::flat_map<MobLogicSystem::ActionEnum, std::u16string>
      actions = {
          {MobLogicSystem::ActionEnum::kStand, u"stand"},
          {MobLogicSystem::ActionEnum::kMove, u"move"},
          {MobLogicSystem::ActionEnum::kFly, u"fly"},
      };

  std::flat_set<MobLogicSystem::ActionEnum> actions2;
  switch (mob.type) {
  case ServerMob::MobType::kStand: {
    if (mob.hate_id != 0) {
      actions2 = {
          MobLogicSystem::ActionEnum::kMove,
      };
    } else {
      actions2 = {
          MobLogicSystem::ActionEnum::kStand,
          MobLogicSystem::ActionEnum::kMove,
      };
    }
    break;
  }
  case ServerMob::MobType::kSwim:
  case ServerMob::MobType::kFly: {
    break;
  }
  }

  // 如果 actions2 为空，直接返回避免 dist 构造失败
  if (actions2.empty()) {
    return;
  }

  auto &gen = RandomGameInstance::gen;
  std::uniform_int_distribution<size_t> dist(0, actions2.size() - 1);
  auto it = std::next(actions2.begin(), dist(gen));
  auto selected = *it;
  mob.action = actions.at(selected);

  switch (selected) {
  case MobLogicSystem::ActionEnum::kStand: {
    mob.duration = Window::dt_now_ + 500;
    mob.hspeed = 0;
    break;
  }
  case MobLogicSystem::ActionEnum::kJump: {
    break;
  }
  case MobLogicSystem::ActionEnum::kMove: {
    bool left = false;
    const auto &clients = ServerClientInstance::clients_;
    if (mob.hate_id != 0 && clients.contains(mob.hate_id)) {
      auto &g_character = clients.at(mob.hate_id).player_t.character;
      auto &g_x = g_character->state->x;
      left = g_x <= mob.pos.x;
    } else {
      std::bernoulli_distribution dist(0.5);
      left = dist(gen);
    }
    mob.hforce = left ? -MOVE_FORCE : MOVE_FORCE;
    mob.flip = left ? false : true;
    mob.hspeed = left == true ? -mob.hspeed_max : mob.hspeed_max;
    mob.duration = Window::dt_now_ + 1000;
    break;
  }
  default: {
    break;
  }
  }
}

void ServerMobSystem::RunDefaultAction(ServerMob &mob) {
  switch (mob.type) {
  case ServerMob::MobType::kStand: {
    if (mob.fh != 0) {
      RunStandAction(mob);
    }
    break;
  }
  case ServerMob::MobType::kSwim: {
    RunSwimAction(mob);
    break;
  }
  case ServerMob::MobType::kFly: {
    RunFlyAction(mob);
    break;
  }
  }
}

void ServerMobSystem::RunFlyAction(ServerMob &mob) {}

void ServerMobSystem::RunSwimAction(ServerMob &mob) {}

void ServerMobSystem::RunHitAction(ServerMob &mob) {
  mob.action = u"hit1";
}

void ServerMobSystem::RunMoveAction(ServerMob &mob) {
  mob.action = u"move";
}

void ServerMobSystem::RunStandAction(ServerMob &mob) {
  mob.action = u"stand";
}

void ServerMobSystem::RunDieAction(ServerMob &mob) {
  mob.action = u"die1";
}

void ServerMobSystem::RunDie(ServerMob &mob, uint64_t client_id) {
  mob.hate_id = 0;
  RunDieAction(mob);
  fbs::ServerMobDieT smd;
  smd.client_id = client_id;
  smd.mob_index = mob.index;

  fbs::MobEventUnionUnion muu;
  muu.Set(std::move(smd));
  events_.payload.push_back(std::move(muu));

  auto mob_drops = ServerMobInstance::LoadMobDrops(mob.id);
  std::vector<fbs::DropT> dts = ServerMobInstance::CreateMobDts(
      mob.id, mob.pos.x, mob.pos.y, mob.page);

  fbs::ServerMobDropT smt;
  smt.mob_index = mob.index;
  smt.drop = ServerDropInstance::CreateDts(dts, map_id_);
  for (const auto &dt : smt.drop) {
    ServerDropInstance::SaveDrop(map_id_, *dt);
  }

  fbs::MobEventUnionUnion mu;
  mu.Set(std::move(smt));
  events_.payload.push_back(std::move(mu));

  mob.debuffs.clear();
  return;
}

bool ServerMobSystem::RunHitting(ServerMob &mob) {
  if (mob.hits.empty()) {
    return false;
  }
  const auto current_time = Window::dt_time_;
  // 1) 结算本帧累积伤害（用 int64_t 防溢出）
  int64_t total_damage = 0;
  for (auto &b : mob.hits | std::views::values) {
    if (b.hit_num != 0) {
      total_damage += b.hit_num;
      b.hit_num = 0;
    }
  }
  mob.hp -= total_damage;
  // 2) 取首/尾（hits 需为有序容器，保证 rbegin 是"最后一次命中"）
  const auto &first = mob.hits.begin()->second;
  const auto &last = mob.hits.rbegin()->second;
  // 首帧未到，暂不推进动作（伤害已结算，保持原语义）
  if (first.hit_time > current_time) {
    return false;
  }
  // 4) 死亡判定：HP 归零且最后一击已过期
  if (mob.hp <= 0 && last.hit_time < current_time) {
    const auto killer_id = last.hit_id; // clear 前保存
    mob.hits.clear();
    RunDie(mob, killer_id);
    return true;
  }
  mob.hate_id = last.hit_id;
  // 5) 推进动作：非击退帧执行 hit action；击退帧弹出队首
  if (first.hit_push) {
    // 3) 更新朝向/击退方向
    mob.hforce = first.left ? MOVE_FORCE : -MOVE_FORCE;
    mob.flip = !first.left;
    RunHitAction(mob);
  } else {
    mob.hits.erase(mob.hits.begin());
  }
  return true;
}

bool ServerMobSystem::RunHitCheck(ServerMob &mob) {
  if (mob.hits.empty()) {
    return false;
  }

  auto it = mob.hits.begin();
  const auto time = it->first;

  // 只读取需要的字段，避免拷贝整个 hit 对象
  if (!it->second.hit_push) {
    mob.hits.erase(it);
    return false;
  }

  const auto delay = LoadMobHitCd(mob);
  if (time + delay > Window::dt_time_) {
    return false;
  }

  mob.hits.erase(it);
  return true;
}

void ServerMobSystem::RunHit(ServerMob &mob) {
  if (RunHitCheck(mob)) {
    RunDefaultAction(mob);
    RunDuration(mob);
  }
  return;
}

bool ServerMobSystem::RunFall(ServerMob &mob) {
  if (mob.fh != 0) {
    return false;
  }

  auto border = MapInfoGameInstance::LoadMrBorder(map_id_);
  border.x = mob.rx0;
  border.w = mob.rx1;
  auto vspeed = mob.vspeed + delta_time_ * 2;
  mob.vspeed = vspeed;

  const float vspeed_min = -5550.0f;
  const float vspeed_max = 670.0f;

  bool r = Physic::Fall(mob.pos, delta_time_ / 1000.0f, mob.hspeed, mob.vspeed,
                        vspeed_min, vspeed_max, border, true, true, mob.fh,
                        mob.page, g_fhs_, INT32_MAX);
  return r;
}

bool ServerMobSystem::RunDebuffAction(ServerMob &mob) {
  if (mob.debuffs.contains(GameDebuff::DebuffType::kDizz)) {
    RunDefaultAction(mob);
    return true;
  }
  return false;
}

void ServerMobSystem::RunStateMachine(ServerMob &mob) {
  auto o_mob = mob; // 拷贝用于对比
  auto m_action = MobLogicSystem::LoadActionType(mob.action);
  switch (m_action) {
  case MobLogicSystem::ActionEnum::kRevive: {
    break;
  }
  case MobLogicSystem::ActionEnum::kDie: {
    break;
  }
  case MobLogicSystem::ActionEnum::kStand: {
    if (!RunHitting(mob)) {
      RunDuration(mob);
    }
    break;
  }
  case MobLogicSystem::ActionEnum::kMove: {
    if (RunDebuffAction(mob)) {
      break;
    }
    if (!RunHitting(mob)) {
      RunWalk(mob);
      if (!RunTryJump(mob)) {
        RunDuration(mob);
      }
    }
    break;
  }
  case MobLogicSystem::ActionEnum::kHit: {
    switch (mob.type) {
    case ServerMob::MobType::kStand: {
      if (mob.fh == 0) {
        if (!RunFall(mob)) {
          RunStandAction(mob);
        }
      } else {
        RunWalk(mob);
      }
      break;
    }
    case ServerMob::MobType::kSwim:
    case ServerMob::MobType::kFly: {
      break;
    }
    }
    RunHit(mob);
    RunHitting(mob);
    break;
  }
  case MobLogicSystem::ActionEnum::kJump: {
    bool hit = RunHitting(mob);
    if (!RunFall(mob)) {
      if (!hit) {
        RunDefaultAction(mob);
      }
    }
    break;
  }
  case MobLogicSystem::ActionEnum::kSwim:
  case MobLogicSystem::ActionEnum::kFly: {
    break;
  }
  }
  RunNetworkSync(mob, o_mob);
}

void ServerMobSystem::RunSend() {
  if (events_.payload.empty()) {
    return;
  }
  events_.map_id = map_id_;
  auto &clients = ServerSceneInstance::scenes_[map_id_].clients;
  for (auto client_id : clients) {
    ServerResponse::send_to_client(client_id, events_);
  }
  events_.payload.clear();
}

bool ServerMobSystem::Run() {
  static uint64_t last = SDL_GetTicks();

  delta_time_ = Window::dt_now_ - last;
  if (delta_time_ < MIN_FRAME_INTERVAL_MS) {
    return true;
  }
  last = Window::dt_now_;
  delta_time_ = std::min(delta_time_, MIN_FRAME_INTERVAL_MS);

  auto &scenes = ServerSceneInstance::scenes_;
  for (auto &sc : scenes | std::views::values) {
    if (sc.mobs.empty() || sc.clients.empty()) {
      continue;
    }
    map_id_ = sc.map_id;
    g_fhs_.clear();
    const auto &s_fhs = ServerSceneInstance::scenes_.at(map_id_).fhs;
    for (const auto &[key, value] : s_fhs) {
      g_fhs_.emplace(key, value.fh);
    }
    for (auto &mob : sc.mobs | std::views::values) {
      RunStateMachine(mob);
    }
    RunSend();
  }
  return true;
}

}  // namespace sdlms
