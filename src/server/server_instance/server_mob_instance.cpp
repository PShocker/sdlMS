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

void ServerMobInstance::load_default_mob(const std::u16string id,
                                           ServerMob &mob) {
  // default action
  mob.id = id;
  auto mob_node = WzResource::mob->find(mob.id + u".img");
  if (mob_node->get_child(u"info")->get_child("flySpeed")) {
    mob.action = u"fly";
    mob.type = ServerMob::MobType::kFly;
  } else {
    mob.action = u"stand";
    mob.type = ServerMob::MobType::kStand;
  }
  mob.duration = Window::dt_now;

  auto info_node = MobGameInstance::load_mob_info(mob.id);
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

void ServerMobInstance::load_mob(ServerSceneState &scene) {
  std::flat_map<uint32_t, ServerMob> data;
  auto map_id = scene.map_id;
  auto map_node = WzResource::load_map_node(map_id);
  auto map_life_node = map_node->get_child(u"life");
  auto fhs = FootholdGameInstance::load(map_id);
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
    load_default_mob(mob.id, mob);
    data[mob.index] = mob;
  }
  scene.mobs = data;
}

void ServerMobInstance::handle_attack(uint64_t client_id,
                                        ClientMobAttackT &r) {
  if (!ServerClientInstance::clients.contains(client_id)) {
    return;
  }
  // 转发
  auto map_id = r.map_id;
  auto clients = ServerSceneInstance::scenes.at(map_id).clients;
  clients.erase(client_id);
  ServerMobAttackT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerMobInstance::handle_server_mv(const ServerMobMvT &m) {
  auto &mvs = MobGameInstance::data.at(m.mob_index).mvs;
  mvs.push_back(*m.payload);
}

void ServerMobInstance::handle_server_flip(const ServerMobFlipT &m) {
  auto &mob = MobGameInstance::data.at(m.mob_index).mob;
  mob.flip = m.payload->flip;
}

void ServerMobInstance::handle_server_action(const ServerMobActionT &m) {
  auto &mob = MobGameInstance::data.at(m.mob_index).mob;
  const auto &a = m.payload;
  mob.action = std::u16string{a->action.begin(), a->action.end()};
  mob.ani_index = 0;
  mob.ani_time = 0;
  auto MobType = MobLogicSystem::load_action_type(mob.action);
  switch (MobType) {
  case MobLogicSystem::ActionEnum::kDie: {
    mob.debuffs = {};
    AudioGameInstance::load_audio(u"Mob.img/" + mob.id + u"/Die", 0);
    break;
  }
  default: {
    break;
  }
  }
}

void ServerMobInstance::handle_server_state(const ServerMobStateT &m) {
  auto &mob = MobGameInstance::data.at(m.mob_index).mob;
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

void ServerMobInstance::handle_server_die(const ServerMobDieT &m) {
  auto &mob = MobGameInstance::data.at(m.mob_index).mob;
  float percent = (float)mob.attack_val / mob.max_hp;
  if (percent) {
    percent = std::min(percent, 1.0f);
    auto info_node = MobGameInstance::load_mob_info(mob.id);
    auto max_exp =
        static_cast<wz::Property<int> *>(info_node->get_child(u"exp"))->get();
    auto exp = percent * max_exp;
    GainLogGameInstance::add(u"", exp, GainEnum::kExperience);
    CharacterStatGameInstance::add_exp(exp);
  }
  if (m.client_id == ServerMain::local_addr) {
    QuestGameInstance::update_check_mob(mob.id, 1);
  } else {
  }
  mob.attack_val = 0;
}

void ServerMobInstance::handle_server_drop(const ServerMobDropT &m) {
  // Drop
  for (const auto &dt : m.drop) {
    ServerDropInstance::handle_server_dt(*dt);
  }
  AudioGameInstance::load_audio(u"Game.img/DropItem", 0);
  return;
}

void ServerMobInstance::handle_server_event(const ServerMobEventT &m) {
  if (m.map_id != SceneSystemInstance::map_id) {
    return;
  }
  for (const auto &ev : m.payload) {
    switch (ev.type) {
    case fbs::MobEventUnion_ServerMobMv: {
      const auto mv = ev.AsServerMobMv();
      handle_server_mv(*mv);
      break;
    }
    case fbs::MobEventUnion_ServerMobFlip: {
      const auto fp = ev.AsServerMobFlip();
      handle_server_flip(*fp);
      break;
    }
    case fbs::MobEventUnion_ServerMobAction: {
      const auto action = ev.AsServerMobAction();
      handle_server_action(*action);
      break;
    }
    case fbs::MobEventUnion_ServerMobState: {
      const auto st = ev.AsServerMobState();
      handle_server_state(*st);
      break;
    }
    case fbs::MobEventUnion_ServerMobDie: {
      const auto d = ev.AsServerMobDie();
      handle_server_die(*d);
      break;
    }
    case fbs::MobEventUnion_ServerMobDrop: {
      const auto d = ev.AsServerMobDrop();
      handle_server_drop(*d);
      break;
    }
    default: {
      break;
    }
    }
  }
}

void ServerMobInstance::handle_s_attack(uint64_t client_id, AttackT &at) {
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
  EffectGameInstance::data[7].emplace_back(d);
}

void ServerMobInstance::handle_server_attack(ServerMobAttackT &r) {
  handle_s_attack(r.client_id, *r.payload);
}

void ServerMobInstance::hanle_server_mob(
    const std::unique_ptr<fbs::MobT> &m) {
  auto &data = MobGameInstance::data;
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
  auto info_node = MobGameInstance::load_mob_info(g_mob.id);
  g_mob.max_hp =
      static_cast<wz::Property<int> *>(info_node->get_child(u"maxHP"))->get();

  g_mob.ani_index = 0;
  g_mob.ani_time = 0;
  g_mob.ani_animate = true;

  auto action_type = MobLogicSystem::load_action_type(g_mob.action);
  if (action_type == MobLogicSystem::ActionEnum::kDie) {
    g_mob.action = u"";
  }

  data[g_mob.index] = {g_mob, {}};
  return;
}

void ServerMobInstance::handle_create_mob(ClientCreateMobT &r) {
  auto map_id = r.map_id;
  auto &scene = ServerSceneInstance::scenes[map_id];
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
    load_default_mob({tmp.begin(), tmp.end()}, mob);
    scene.mobs[mob.index] = mob;

    r.mobs[i]->mob_index = mob.index;
    r.mobs[i]->mob_hp = mob.hp;
    r.mobs[i]->state->action = {mob.action.begin(), mob.action.end()};
    i++;
  }
  ServerCreateMobT sct;
  sct.mobs = std::move(r.mobs);
  for (auto client_id : scene.clients) {
    ServerResponse::send_to_client(client_id, sct);
  }
  return;
}

void ServerMobInstance::handle_s_create_mob(ServerCreateMobT &r) {
  for (const auto &m : r.mobs) {
    hanle_server_mob(m);
  }
  return;
}

std::vector<ServerMobInstance::MobDrop>
ServerMobInstance::load_mob_drops(const std::u16string &mob_id) {
  using CacheType = std::flat_map<std::u16string, std::vector<MobDrop>>;
  static CacheType cache;

  // 使用 try_emplace 一次完成查找和插入
  auto [it, inserted] = cache.try_emplace(mob_id);
  if (!inserted) {
    return it->second;
  }

  auto node = WzResource::ms->get_root()->find(u"MobDrop.img/" + mob_id);
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

std::vector<DropT>
ServerMobInstance::create_mob_dts(const std::u16string &mob_id, float x,
                                    float y, int page) {
  auto mob_drops = load_mob_drops(mob_id);
  std::vector<DropT> dts;
  for (const auto &drop : mob_drops) {
    auto &gen = RandomGameInstance::gen;
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    bool success = dis(gen) <= drop.rate;
    if (!success) {
      continue;
    }
    DropT dt;
    dt.x1 = x;
    dt.y1 = y;
    dt.page = page;
    if (ItemGameInstance::check_item(drop.id)) {
      auto min_quantity = drop.min_quantity;
      auto max_quantity = drop.max_quantity;
      std::uniform_int_distribution<int> dis(min_quantity, max_quantity);
      int random_num = dis(gen);
      ItemT it;
      it.item_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
      it.item_num = random_num;
      dt.drop.Set(it);
    } else {
      EquipT et;
      et.equip_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
      dt.drop.Set(et);
    }
    dts.push_back(dt);
  }
  return dts;
}

void ServerMobInstance::handle_debuff(uint64_t client_id,
                                        ClientMobDebuffT &r) {
  auto map_id = r.map_id;
  auto &scene = ServerSceneInstance::scenes[map_id];
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
  ServerMobDebuffT smd;
  smd.payload = std::move(r.payload);
  for (auto c : scene.clients) {
    ServerResponse::send_to_client(c, smd);
  }
  return;
}

void ServerMobInstance::handle_s_debuff(ServerMobDebuffT &r) {
  auto &Mobs = MobGameInstance::data;
  for (const auto &db : r.payload) {
    if (db->payload->time <= Window::dt_time) {
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