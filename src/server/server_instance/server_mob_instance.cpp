#include "server_mob_instance.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "src/server/server_main.h"
#include "src/server/server_system/server_mob_system.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <memory>
#include <optional>
namespace sdlms {


void ServerMobInstance::LoadDefaultMob(const std::u16string id,
                                           ServerMob &mob) {
  // default action
  mob.id = id;
  auto mob_node = WzResource::mob_->find(mob.id + u".img");
  if (mob_node->get_child(u"info")->get_child("flySpeed")) {
    mob.action = u"fly";
    mob.type = ServerMob::MobType::kFly;
  } else {
    mob.action = u"stand";
    mob.type = ServerMob::MobType::kStand;
  }
  mob.duration = Window::dt_now_;

  auto info_node = MobGameInstance::LoadMobInfo(mob.id);
  mob.hp =
      static_cast<wz::Property<int> *>(info_node->get_child(u"maxHP"))->get();
  mob.mp =
      static_cast<wz::Property<int> *>(info_node->get_child(u"maxMP"))->get();
  if (info_node->get_child(u"speed") != nullptr) {
    auto speed =
        static_cast<wz::Property<int> *>(info_node->get_child(u"speed"))->get();
    mob.hspeed_max = (float)(speed + 100) / 100 * 125;
  }
}

void ServerMobInstance::LoadMob(ServerSceneState &scene) {
  std::flat_map<uint32_t, ServerMob> data;
  auto map_id = scene.map_id;
  auto map_node = WzResource::LoadMapNode(map_id);
  auto map_life_node = map_node->get_child(u"life");
  auto fhs = FootholdGameInstance::Load(map_id);
  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string> *>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"n") {
      continue;
    }
    ServerMob mob;
    auto mob_node = val[0];
    mob.index = std::stoi(std::string{key.begin(), key.end()});
    mob.id =
        static_cast<wz::Property<std::u16string> *>(mob_node->get_child(u"id"))
            ->get();
    mob.fh =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"fh"))->get();
    mob.rx0 =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"rx0"))->get();
    mob.rx1 =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"rx1"))->get();

    auto x = static_cast<wz::Property<int> *>(mob_node->get_child(u"x"))->get();
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
                                        fbs::ClientMobAttackT &r) {
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
    ServerResponse::send_to_client(c, t);
  }
}

void ServerMobInstance::HandleServerMv(const fbs::ServerMobMvT &m) {
  auto &mvs = MobGameInstance::data_.at(m.mob_index).mvs;
  mvs.push_back(*m.payload);
}

void ServerMobInstance::HandleServerFlip(const fbs::ServerMobFlipT &m) {
  auto &mob = MobGameInstance::data_.at(m.mob_index).mob;
  mob.flip = m.payload->flip;
}

void ServerMobInstance::HandleServerAction(const fbs::ServerMobActionT &m) {
  auto &mob = MobGameInstance::data_.at(m.mob_index).mob;
  const auto &a = m.payload;
  mob.action = std::u16string{a->action.begin(), a->action.end()};
  mob.ani_index = 0;
  mob.ani_time = 0;
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

void ServerMobInstance::HandleServerState(const fbs::ServerMobStateT &m) {
  auto &mob = MobGameInstance::data_.at(m.mob_index).mob;
  for (const auto &s : m.payload) {
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

void ServerMobInstance::HandleServerDie(const fbs::ServerMobDieT &m) {
  auto &mob = MobGameInstance::data_.at(m.mob_index).mob;
  float percent = (float)mob.attack_val / mob.max_hp;
  if (percent) {
    percent = std::min(percent, 1.0f);
    auto info_node = MobGameInstance::LoadMobInfo(mob.id);
    auto max_exp =
        static_cast<wz::Property<int> *>(info_node->get_child(u"exp"))->get();
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

void ServerMobInstance::HandleServerDrop(const fbs::ServerMobDropT &m) {
  // Drop
  for (const auto &dt : m.drop) {
    ServerDropInstance::HandleServerDt(*dt);
  }
  AudioGameInstance::LoadAudio(u"Game.img/DropItem", 0);
  return;
}

void ServerMobInstance::HandleServerEvent(const fbs::ServerMobEventT &m) {
  if (m.map_id != SceneSystemInstance::map_id_) {
    return;
  }
  for (const auto &ev : m.payload) {
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
    default: {
      break;
    }
    }
  }
}

void ServerMobInstance::HandleSAttack(uint64_t client_id, fbs::AttackT &at) {
  const auto &attack = at;
  // 伤害数字
  DamageData data = {
      .num = attack.num,
      .type = (DamageData::T)attack.type,
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

void ServerMobInstance::HandleServerAttack(fbs::ServerMobAttackT &r) {
  HandleSAttack(r.client_id, *r.payload);
}

void ServerMobInstance::HanleServerMob(
    const std::unique_ptr<fbs::MobT> &m) {
  auto &data = MobGameInstance::data_;
  GameMob g_mob;
  auto tmp = std::format("{:07d}", m->mob_id);
  g_mob.id = {tmp.begin(), tmp.end()};
  g_mob.index = m->mob_index;
  const auto &state = m->state;
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
      static_cast<wz::Property<int> *>(info_node->get_child(u"maxHP"))->get();

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

void ServerMobInstance::HandleCreateMob(fbs::ClientCreateMobT &r) {
  auto map_id = r.map_id;
  auto &scene = ServerSceneInstance::scenes_[map_id];
  int i = 0;
  for (const auto &mb : r.mobs) {
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
    ServerResponse::send_to_client(client_id, sct);
  }
  return;
}

void ServerMobInstance::HandleSCreateMob(fbs::ServerCreateMobT &r) {
  for (const auto &m : r.mobs) {
    HanleServerMob(m);
  }
  return;
}

std::vector<ServerMobInstance::MobDrop>
ServerMobInstance::LoadMobDrops(const std::u16string &mob_id) {
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

  auto &drops = it->second;
  auto *children = node->get_children();
  drops.reserve(children ? children->size() : 0);

  if (!children) {
    return drops;
  }

  for (auto [k, v] : *children) {
    if (v.empty())
      continue;

    auto *child = v[0];
    if (!child)
      continue;

    auto *min_quantity_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"min_quantity"));
    auto *max_quantity_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"max_quantity"));
    auto *chance_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"chance"));

    if (!min_quantity_prop || !max_quantity_prop || !chance_prop) {
      continue;
    }

    MobDrop md;
    md.id = k; // 如果后续不需要原字符串，可改为 std::move(k)
    md.min_quantity = min_quantity_prop->get();
    md.max_quantity = max_quantity_prop->get();
    md.rate = chance_prop->get() / 1000000.0f;
    drops.push_back(std::move(md));
  }

  return drops;
}

std::vector<fbs::DropT>
ServerMobInstance::CreateMobDts(const std::u16string &mob_id, float x,
                                    float y, int page) {
  auto mob_drops = LoadMobDrops(mob_id);
  std::vector<fbs::DropT> dts;
  for (const auto &drop : mob_drops) {
    auto &gen = RandomGameInstance::gen;
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    bool success = dis(gen) <= drop.rate;
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
      int random_num = dis(gen);
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
                                        fbs::ClientMobDebuffT &r) {
  auto map_id = r.map_id;
  auto &scene = ServerSceneInstance::scenes_[map_id];
  auto &Mobs = scene.mobs;
  for (auto &db : r.payload) {
    if (Mobs.contains(db->mob_index)) {
      auto &mb = Mobs.at(db->mob_index);
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
    ServerResponse::send_to_client(c, smd);
  }
  return;
}

void ServerMobInstance::HandleSDebuff(fbs::ServerMobDebuffT &r) {
  auto &Mobs = MobGameInstance::data_;
  for (const auto &db : r.payload) {
    if (db->payload->time <= Window::dt_time_) {
      continue;
    }
    if (Mobs.contains(db->mob_index)) {
      auto &mb = Mobs.at(db->mob_index).mob;
      GameDebuff debuff;
      debuff.type = (GameDebuff::DebuffType)db->payload->val;
      debuff.delay = db->payload->delay;
      debuff.destroy = db->payload->time;
      mb.debuffs[debuff.type] = debuff;
    }
  }
}

}  // namespace sdlms
