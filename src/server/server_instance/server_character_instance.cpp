#include "server_character_instance.h"
#include "SDL3/SDL_rect.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_chat.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/chat_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include "src/server/server_main.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_set>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <utility>

using namespace fbs;

void ServerCharacterInstance::handle_mv(uint64_t client_id,
                                          ClientCharacterMvT &m) {
  if (!ServerClientInstance::clients.contains(client_id)) {
    return;
  }
  auto &client = ServerClientInstance::clients.at(client_id);
  auto &character = client.player_t.character;

  character->state->x = m.payload->x2;
  character->state->y = m.payload->y2;
  character->state->page = m.payload->page;

  const auto client_map_id = m.map_id;
  auto clients = ServerSceneInstance::scenes[client_map_id].clients;
  // 移除当前客户端，只发给其他客户端
  clients.erase(client_id);
  ServerCharacterMvT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerCharacterInstance::handle_flip(uint64_t client_id,
                                            ClientCharacterFlipT &m) {
  if (!ServerClientInstance::clients.contains(client_id)) {
    return;
  }
  auto &client = ServerClientInstance::clients.at(client_id);
  auto &character = client.player_t.character;

  character->state->flip = m.payload->flip;

  const auto client_map_id = m.map_id;
  auto clients = ServerSceneInstance::scenes[client_map_id].clients;
  clients.erase(client_id);
  ServerCharacterFlipT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerCharacterInstance::handle_action(uint64_t client_id,
                                              ClientCharacterActionT &m) {
  if (!ServerClientInstance::clients.contains(client_id)) {
    return;
  }
  auto &client = ServerClientInstance::clients.at(client_id);
  auto &character = client.player_t.character;

  const static std::flat_set<std::string> actions = {
      "stand1", "stand2", "alert",  "walk1", "walk2",
      "prone",  "jump",   "ladder", "rope",  "dead",
  };
  const auto &a = m.payload;
  if (actions.contains(a->action)) {
    character->state->action = a->action;
    character->state->action_index = a->action_index;
    character->state->action_animate = a->action_animate;
  }

  const auto client_map_id = m.map_id;
  auto clients = ServerSceneInstance::scenes[client_map_id].clients;
  clients.erase(client_id);
  ServerCharacterActionT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerCharacterInstance::handle_die(uint64_t client_id,
                                           ClientCharacterDieT &m) {
  if (!ServerClientInstance::clients.contains(client_id)) {
    return;
  }
  auto &client = ServerClientInstance::clients.at(client_id);
  auto &character = client.player_t.character;

  character->state->x = m.payload->x;
  character->state->y = m.payload->y;
  character->state->action = "dead";
  character->state->action_index = 0;
  character->state->action_animate = true;

  const auto client_map_id = m.map_id;
  auto clients = ServerSceneInstance::scenes[client_map_id].clients;
  clients.erase(client_id);
  ServerCharacterDieT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerCharacterInstance::handle_fc(uint64_t client_id,
                                          ClientCharacterFcT &m) {
  if (!ServerClientInstance::clients.contains(client_id)) {
    return;
  }
  auto &client = ServerClientInstance::clients.at(client_id);
  auto &character = client.player_t.character;

  character->face->face_action = m.payload->face_action;

  const auto client_map_id = m.map_id;
  auto clients = ServerSceneInstance::scenes[client_map_id].clients;
  clients.erase(client_id);
  ServerCharacterFcT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerCharacterInstance::handle_server_mv(uint64_t client_id,
                                                 ServerCharacterMvT &m) {
  if (!CharacterGameInstance::others.contains(m.client_id)) {
    return;
  }
  auto &mvs = CharacterGameInstance::others[m.client_id].mvs;
  mvs.push_back(*m.payload);
}

void ServerCharacterInstance::handle_server_flip(uint64_t client_id,
                                                   ServerCharacterFlipT &m) {
  if (!CharacterGameInstance::others.contains(m.client_id)) {
    return;
  }
  auto &c = CharacterGameInstance::others[m.client_id].g_character;
  c.flip = m.payload->flip;
}

void ServerCharacterInstance::handle_server_action(
    uint64_t client_id, ServerCharacterActionT &m) {
  if (!CharacterGameInstance::others.contains(m.client_id)) {
    return;
  }
  auto &c = CharacterGameInstance::others[m.client_id].g_character;
  const auto &a = m.payload;
  c.action = std::u16string{a->action.begin(), a->action.end()};
  c.action_index = a->action_index;
  c.action_animate = a->action_animate;
  c.action_time = 0;
  auto action = CharacterLogicSystem::load_action_type(c);
  switch (action) {
  case CharacterLogicSystem::ActionEnum::jump:
  case CharacterLogicSystem::ActionEnum::stand: {
    c.skill = std::nullopt;
    break;
  }
  case CharacterLogicSystem::ActionEnum::skill:
  case CharacterLogicSystem::ActionEnum::attack: {
    CharacterLogicSystem::load_sfx(c);
    break;
  }
  case CharacterLogicSystem::ActionEnum::dead: {
    AudioGameInstance::load_audio(u"Game.img/Tombstone", 0);
    break;
  }
  default: {
    break;
  }
  }
}

void ServerCharacterInstance::handle_server_die(uint64_t client_id,
                                                  ServerCharacterDieT &m) {
  if (!CharacterGameInstance::others.contains(m.client_id)) {
    return;
  }
  auto &c = CharacterGameInstance::others[m.client_id].g_character;
  const auto &d = m.payload;
  c.pos.x = d->x;
  c.pos.y = d->y;
  c.action = u"dead";
  c.action_animate = true;
  c.action_index = 0;
  c.action_time = 0;
  GameTomb t{
      .ani_type = u"fall",
      .ani_index = 0,
      .ani_time = 0,
      .pos = {d->x, d->y - 300},
      .b = c.pos,
  };
  c.tomb = t;
}

void ServerCharacterInstance::handle_server_fc(uint64_t client_id,
                                                 ServerCharacterFcT &m) {
  if (!CharacterGameInstance::others.contains(m.client_id)) {
    return;
  }
  const auto &f = m.payload;
  auto &c = CharacterGameInstance::others[m.client_id].g_character;
  c.face.action = std::u16string{f->face_action.begin(), f->face_action.end()};
  c.face.index = 0;
  c.face.time = 0;
  if (c.face.action == u"default") {
    c.face.destroy = Window::dt_now + 4000;
  } else {
    c.face.destroy = UINT64_MAX;
  }
}

void ServerCharacterInstance::handle_skill(uint64_t client_id,
                                             ClientCharacterSkillT &r) {
  if (ServerClientInstance::clients.contains(client_id)) {
    auto map_id = ServerClientInstance::clients.at(client_id).map_id;
    auto scenes = ServerSceneInstance::scenes[map_id].clients;
    scenes.erase(client_id);
    ServerCharacterSkillT t;
    t.client_id = client_id;
    t.ski_id = r.ski_id;
    t.ski_lv = r.ski_lv;
    t.payload = std::move(r.payload);
    t.x = r.x;
    t.y = r.y;
    t.flip = r.flip;
    for (auto c : scenes) {
      ServerResponse::send_to_client(c, t);
    }
  }
}

void ServerCharacterInstance::handle_attack(uint64_t client_id,
                                              ClientCharacterAttackT &r) {
  if (!ServerClientInstance::clients.contains(client_id)) {
    return;
  }
  auto map_id = ServerClientInstance::clients.at(client_id).map_id;
  auto &Mobs = ServerSceneInstance::scenes.at(map_id).mobs;
  for (const auto &a : r.payload) {
    auto &mob = Mobs.at(a->mob_index);
    auto mob_info = MobGameInstance::load_mob_info(mob.id);
    auto pushed =
        static_cast<wz::Property<int> *>(mob_info->get_child(u"pushed"))->get();
    MobHit mh;
    mh.left = a->left;
    mh.hit_id = client_id;
    mh.hit_time = a->attack->delay;
    mh.hit_num = a->attack->num;
    mh.hit_push = mh.hit_num >= pushed;
    mob.hits.emplace(mh.hit_time, mh);
  }
  // 转发
  const auto &clients = ServerSceneInstance::scenes.at(map_id).clients;
  ServerCharacterAttackT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerCharacterInstance::handle_chat(uint64_t client_id,
                                            ClientCharacterChatT &r) {
  // 转发
  auto client_name =
      ServerClientInstance::clients.at(client_id).player_t.character->name;
  const auto &clients = ServerSceneInstance::scenes.at(r.map_id).clients;
  ServerCharacterChatT t;
  t.name = client_name;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerCharacterInstance::handle_server_chat(uint64_t client_id,
                                                   ServerCharacterChatT &r) {
  GameCharacter *g_character = nullptr;
  if (r.client_id == ServerMain::local_addr) {
    g_character = &CharacterGameInstance::self;
  } else if (CharacterGameInstance::others.contains(r.client_id)) {
    g_character = &CharacterGameInstance::others[r.client_id].g_character;
  }
  if (g_character) {
    GameChatballoon c;
    c.w = 100;
    c.destroy = Window::dt_now + 5000;
    c.text = {r.payload->payload.begin(), r.payload->payload.end()};
    c.size = 13;
    c.path = u"0";
    g_character->chatballoon = c;
  }

  ChatGameInstance::chats.push_back({
      .type = (GameChatEnum)r.payload->type,
      .owner = {r.name.begin(), r.name.end()},
      .text = {r.payload->payload.begin(), r.payload->payload.end()},
  });
}

void ServerCharacterInstance::handle_character(uint64_t client_id,
                                                 ClientCharacterT &r) {
  if (ServerClientInstance::clients.contains(client_id)) {
    auto map_id = r.map_id;
    auto scenes = ServerSceneInstance::scenes[map_id].clients;
    scenes.erase(client_id);
    ServerCharacterT t;
    t.client_id = client_id;
    t.payload = std::move(r.payload);
    for (auto c : scenes) {
      ServerResponse::send_to_client(c, t);
    }
    // save
    ServerClientInstance::clients[client_id].player_t.character =
        std::move(t.payload);
  }
}

void ServerCharacterInstance::handle_server_atk(uint64_t client_id,
                                                  ServerCharacterAttackT &r) {
  auto &Mobs = MobGameInstance::data;
  std::flat_multiset<uint32_t> MobHit;
  auto &v = r.payload;
  for (uint32_t i = 0; i < v.size(); i++) {
    auto &ct = v[i];
    auto &mob = Mobs[ct->mob_index].mob;
    if (!ct->effect.empty()) {
      GameEffect e = {
          .id = {ct->effect.begin(), ct->effect.end()},
          .index = 0,
          .time = 0,
          .delay = ct->attack->delay,
          .type = GameEffect::EffectType::effect,
          .pos = SDL_FPoint{ct->attack->x, ct->attack->y},
          .z = false,
      };
      if (WzResource::item->find(e.id)) {
        e.lv = GameEffect::EffectSubType::item;
      } else if (WzResource::skill->find(e.id)) {
        e.lv = GameEffect::EffectSubType::skill;
      }
      mob.effect.push_back(e);
    }
    // 声音
    if (!ct->sound.empty()) {
      std::u16string tmp{ct->sound.begin(), ct->sound.end()};
      AudioGameInstance::load_audio(tmp, ct->attack->delay);
    }
    // 伤害数字
    DamageData data = {
        .num = ct->attack->num,
        .type = DamageData::r,
    };
    GameEffect d = {
        .id = u"",
        .index = (uint32_t)MobHit.count(mob.index),
        .time = mob.index,
        .delay = ct->attack->delay,
        .type = GameEffect::EffectType::damage,
        .pos = SDL_FPoint{ct->attack->x, ct->attack->y - 10},
        .z = false,
        .flip = false,
        .data = data,
    };
    if (r.client_id == ServerMain::local_addr) {
      mob.attack_val += data.num;
    }
    std::u16string path = u"Mob.img/" + mob.id + u"/Damage";
    AudioGameInstance::load_audio(path, ct->attack->delay);
    MobHit.insert(mob.index);
    EffectGameInstance::data[7].emplace_back(d);
  }
}

void ServerCharacterInstance::handle_ski(
    uint32_t ski_id, uint8_t ski_lv,
    const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
    uint64_t client_id) {
  if (CharacterGameInstance::others.contains(client_id)) {
    auto g_character = &CharacterGameInstance::others[client_id].g_character;
    auto ski_id2 = std::to_string(ski_id);
    auto ski_id3 = std::u16string{ski_id2.begin(), ski_id2.end()};
    if (ski_id3.length() < 7) {
      ski_id3.insert(0, 7 - ski_id3.length(), u'0');
    }
    g_character->skill = ski_id3;
  }
  handle_ski(ski_id, ski_lv, v, client_id, INT32_MAX, INT32_MAX, false);
}

void ServerCharacterInstance::handle_ski(
    uint32_t ski_id, uint8_t ski_lv,
    const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
    uint64_t client_id, int32_t x, int32_t y, bool flip) {
  GameCharacter *g_character;
  if (client_id == 0) {
    g_character = &CharacterGameInstance::self;
  } else if (CharacterGameInstance::others.contains(client_id)) {
    g_character = &CharacterGameInstance::others[client_id].g_character;
  } else {
    return;
  }
  auto ski_id2 = std::to_string(ski_id);
  auto ski_id3 = std::u16string{ski_id2.begin(), ski_id2.end()};

  if (ski_id3.length() < 7) {
    ski_id3.insert(0, 7 - ski_id3.length(), u'0');
  }

  g_character->skill = ski_id3;
  auto skis = SkillGameInstance::skis();

  if (skis.at(ski_id3).effect) {
    GameEffect e = {
        .id = ski_id3,
        .index = 0,
        .time = 0,
        .delay = Window::dt_now,
        .type = GameEffect::EffectType::skill_custom,
        .pos = SDL_FPoint{(float)x, (float)y},
        .z = false,
        .flip = flip,
        .data = g_character,
    };
    auto &eff = g_character->effect;
    std::erase_if(eff, [&](const auto &ef) { return ef.id == e.id; });
    eff.push_back(e);
  } else {
    auto ski_node = SkillGameInstance::load_ski_node(ski_id3);

    const bool has_position = (x != INT32_MAX || y != INT32_MAX);
    const bool has_z = static_cast<bool>(ski_node->find(u"effect/z"));

    if (ski_node->get_child(u"effect")) {
      GameEffect e = {
          .id = ski_id3,
          .index = 0,
          .time = 0,
          .delay = 0,
          .type = GameEffect::EffectType::skill_use,
          .pos = std::nullopt,
          .z = has_z,
          .order = std::nullopt,
          .data = g_character,
      };
      if (has_position) {
        e.pos = {static_cast<float>(x), static_cast<float>(y)};
        e.flip = flip;
      }
      g_character->effect.push_back(e);
    }
    for (int i = 0; ski_node->get_child("effect/" + std::to_string(i)); i++) {
      GameEffect e = {
          .id = ski_id3,
          .index = 0,
          .time = 0,
          .delay = 0,
          .type = GameEffect::EffectType::skill_use,
          .pos = std::nullopt,
          .z = has_z,
          .order = i,
          .data = g_character,
      };
      if (has_position) {
        e.pos = {static_cast<float>(x), static_cast<float>(y)};
        e.flip = flip;
      }
      g_character->effect.push_back(e);
    }
  }

  auto sound_path = u"Skill.img/" + ski_id3 + u"/Use";
  auto audio_node = WzResource::sound->find(sound_path);
  if (audio_node) {
    AudioGameInstance::load_audio(sound_path, 0);
  }

  auto &mob = MobGameInstance::data;
  for (const auto &s : v) {
    if (s->player == 0) {
      GameEffect e2 = {
          .id = ski_id3,
          .index = 0,
          .time = 0,
          .delay = s->delay,
          .lv = ski_lv,
          .type = GameEffect::EffectType::skill_hit,
          .pos = SDL_FPoint{s->x, s->y},
          .z = false,
      };
      if (mob.contains(s->mob)) {
        mob[s->mob].mob.effect.push_back(e2);
      } else {
        e2.flip = flip;
        auto page = g_character->page;
        EffectGameInstance::data[page].emplace_back(e2);
      }
    } else {
      GameCharacter *c = nullptr;
      if (CharacterGameInstance::others.contains(s->player)) {
        c = &CharacterGameInstance::others[s->player].g_character;
      } else if (s->player == ServerMain::local_addr) {
        c = &CharacterGameInstance::self;
        // ski_buff
        auto &skis = SkillGameInstance::skis();
        skis.at(ski_id3).use(client_id, ski_lv);
      }
      if (c != nullptr) {
        GameEffect e = {
            .id = ski_id3,
            .index = 0,
            .time = 0,
            .delay = 0,
            .type = GameEffect::EffectType::skill_affected,
            .pos = std::nullopt,
            .z = false,
            .data = g_character,
        };
        c->effect.push_back(e);
      }
    }
  }
}

void ServerCharacterInstance::handle_server_ski(uint64_t client_id,
                                                  ServerCharacterSkillT &r) {
  handle_ski(r.ski_id, r.ski_lv, r.payload, r.client_id, r.x, r.y, r.flip);
}

void ServerCharacterInstance::load_g_character(
    GameCharacter &g_character, const std::unique_ptr<CharacterT> &c) {
  const auto &appearance = c->appearance;

  std::string tmp = std::format("{:08d}", appearance->body);
  CharacterGameInstance::add_body(g_character, {tmp.begin(), tmp.end()});

  tmp = std::format("{:08d}", appearance->head);
  CharacterGameInstance::add_head(g_character, {tmp.begin(), tmp.end()});

  tmp = std::format("{:08d}", appearance->face);
  CharacterGameInstance::add_face(g_character, {tmp.begin(), tmp.end()});

  tmp = std::format("{:08d}", appearance->hair);
  CharacterGameInstance::add_hair(g_character, {tmp.begin(), tmp.end()});

  tmp = appearance->ear;
  CharacterGameInstance::add_ear(g_character, {tmp.begin(), tmp.end()});

  g_character.action = {
      c->state->action.begin(),
      c->state->action.end(),
  };
  g_character.action_index = c->state->action_index;
  g_character.action_time = 0;
  g_character.action_animate = c->state->action_animate;
  g_character.page = c->state->page;
  g_character.flip = c->state->flip;

  g_character.pos = SDL_FPoint{
      c->state->x,
      c->state->y,
  };

  g_character.fame = c->fame;
  g_character.level = c->level;
  g_character.job = {c->job.begin(), c->job.end()};
  g_character.face.action = {
      c->face->face_action.begin(),
      c->face->face_action.end(),
  };
  g_character.name = {c->name.begin(), c->name.end()};

  g_character.weapon = std::nullopt;
  g_character.weapon_deco = std::nullopt;

  g_character.cap = std::nullopt;
  g_character.cap_deco = std::nullopt;

  g_character.cape = std::nullopt;
  g_character.cape_deco = std::nullopt;

  g_character.coat = std::nullopt;
  g_character.coat_deco = std::nullopt;

  g_character.glove = std::nullopt;
  g_character.glove_deco = std::nullopt;

  g_character.pant = std::nullopt;
  g_character.pant_deco = std::nullopt;

  g_character.ring0_deco = std::nullopt;
  g_character.ring1_deco = std::nullopt;

  g_character.shield = std::nullopt;
  g_character.shield_deco = std::nullopt;

  g_character.longcoat = std::nullopt;
  g_character.longcoat_deco = std::nullopt;

  g_character.shoes = std::nullopt;
  g_character.shoes_deco = std::nullopt;

  g_character.accessory = std::nullopt;
  g_character.accessory_deco = std::nullopt;

  for (auto &equip : c->equips) {
    tmp = std::format("{:08d}", equip->equip_id);
    GameEquipItem e;
    e.id = {tmp.begin(), tmp.end()};
    for (const auto &scroll : equip->scroll) {
      tmp = std::format("{:08d}", scroll->scroll_id);
      e.scroll.push_back({{tmp.begin(), tmp.end()}, scroll->success});
    }
    EquipGameInstance::add_equip(e, g_character, -1);
  }
  for (auto &deco : c->decos) {
    tmp = std::format("{:08d}", deco->deco_id);
    GameDecoItem e;
    e.id = {tmp.begin(), tmp.end()};
    EquipGameInstance::add_equip_deco(e, g_character, -1);
  }
}

GameCharacter ServerCharacterInstance::load_g_character(
    const std::unique_ptr<CharacterT> &c) {
  GameCharacter g_character;
  load_g_character(g_character, c);
  return g_character;
}

CharacterT ServerCharacterInstance::load_charactert(const GameCharacter &g) {
  fbs::CharacterT c;

  c.appearance = std::make_unique<fbs::CharacterAppearanceT>();
  c.state = std::make_unique<fbs::LifeStateT>();
  c.face = std::make_unique<fbs::FaceT>();

  auto equips = EquipGameInstance::load_equips(g);
  for (const auto &eq : equips) {
    auto id = eq.id;
    EquipT et;
    et.equip_id = std::stoi(std::string{id.begin(), id.end()});
    for (const auto &scroll : eq.scroll) {
      EquipScrollT est;
      est.scroll_id =
          std::stoi(std::string{scroll.id.begin(), scroll.id.end()});
      est.success = scroll.success;
      et.scroll.push_back(std::make_unique<EquipScrollT>(est));
    }
    c.equips.push_back(std::make_unique<EquipT>(et));
  }

  auto decos = EquipGameInstance::load_decos(g);
  for (const auto &de : decos) {
    DecoT dt;
    dt.deco_id = std::stoi(std::string{de.id.begin(), de.id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }

  c.appearance->head = std::stoi(std::string{g.head.begin(), g.head.end()});
  c.appearance->body = std::stoi(std::string{g.body.begin(), g.body.end()});

  c.appearance->hair = std::stoi(std::string{g.hair.begin(), g.hair.end()});

  c.appearance->face =
      std::stoi(std::string{g.face.id.begin(), g.face.id.end()});

  c.appearance->ear = std::string{g.ear.begin(), g.ear.end()};

  c.state->action = std::string{g.action.begin(), g.action.end()};
  c.state->x = g.pos.x;
  c.state->y = g.pos.y;
  c.state->page = g.page;

  c.state->flip = g.flip;

  c.state->action_animate = g.action_animate;

  auto name = g.name;
  c.name = {name.begin(), name.end()};

  c.fame = g.fame;
  c.face->face_action = std::string{g.face.action.begin(), g.face.action.end()};
  c.job = std::string{g.job.begin(), g.job.end()};
  c.level = g.level;

  return c;
}

void ServerCharacterInstance::handle_server_playert(
    const std::unique_ptr<fbs::PlayerT> &c) {
  auto g_character = load_g_character(c->character);

  if (g_character.action == u"dead") {
    GameTomb t{
        .ani_type = u"land",
        .ani_index = 0,
        .ani_time = 0,
        .pos = {g_character.pos.x, g_character.pos.y},
        .b = g_character.pos,
    };
    g_character.tomb = t;
  }
  CharacterOtherData cod;
  cod.g_character = g_character;
  cod.player_t = *c;

  auto it =
      CharacterGameInstance::others.emplace(c->client_id, std::move(cod));
  auto &character = it.first->second.g_character;
  ServerCharacterInstance::handle_s_state(character, c->character->states);
}

void ServerCharacterInstance::handle_state(uint64_t client_id,
                                             ClientCharacterStateT &r) {
  if (ServerClientInstance::clients.contains(client_id)) {
    auto &c = ServerClientInstance::clients.at(client_id).player_t.character;
    for (const auto &st : r.payload) {
      handle_save_state(*st, *c);
    }
    auto scenes = ServerSceneInstance::scenes[r.map_id].clients;
    scenes.erase(client_id);
    ServerCharacterStateT t;
    t.client_id = client_id;
    t.payload = std::move(r.payload);
    for (auto c : scenes) {
      ServerResponse::send_to_client(c, t);
    }
  }
}

void ServerCharacterInstance::handle_buff_item(GameCharacter &g_character,
                                                 const StateT &st) {
  auto tmp = std::format("{:08d}", st.val);
  std::u16string itm_id{tmp.begin(), tmp.end()};
  auto item_type = ItemGameInstance::load_item_type(itm_id);
  if (item_type == u"Ins") {
    if (itm_id.starts_with(u"0301")) {
      if (st.sub_val == 1) {
        CharacterLogicSystem::run_sit_chair(g_character, itm_id);
      } else {
        CharacterLogicSystem::run_unsit_chair(g_character);
      }
    }
  } else if (item_type == u"Consume") {
    if (itm_id.starts_with(u"0221")) {
      if (st.sub_val == 1) {
        ItemGameInstance::use_morph_item(itm_id, g_character);
      } else {
        ItemGameInstance::unuse_morph_item(itm_id, g_character);
      }
    }
  }
  return;
}

void ServerCharacterInstance::handle_scroll_use(GameCharacter &g_character,
                                                  bool success) {
  GameEffect e = {
      .id = u"BasicEff.img/Enchant/Success",
      .index = 0,
      .time = 0,
      .delay = 0,
      .lv = GameEffect::EffectSubType::effect,
      .type = GameEffect::EffectType::effect,
      .pos = SDL_FPoint{0, 0},
      .z = false,
  };
  if (!success) {
    e.id = u"BasicEff.img/Enchant/Failure";
  }
  g_character.effect.emplace_back(e);
}

void ServerCharacterInstance::handle_morph_use(GameCharacter &g_character,
                                                 bool success) {
  std::u16string path = u"BasicEff.img/Transform";
  auto type = CharacterLogicSystem::load_action_type(g_character);
  if (type == CharacterLogicSystem::ActionEnum::climb) {
    path = u"BasicEff.img/TransformOnLadder";
  }
  GameEffect e = {
      .id = path,
      .index = 0,
      .time = 0,
      .delay = 0,
      .lv = GameEffect::EffectSubType::effect,
      .type = GameEffect::EffectType::effect,
      .pos = SDL_FPoint{0, 0},
      .z = false,
  };
  g_character.effect.emplace_back(e);
}

void ServerCharacterInstance::handle_item_use(GameCharacter &g_character,
                                                const StateT &st) {
  auto tmp = std::format("{:08d}", st.val);
  std::u16string itm_id{tmp.begin(), tmp.end()};
  auto item_type = ItemGameInstance::load_item_type(itm_id);
  if (item_type == u"Consume") {
    if (itm_id.starts_with(u"0204")) {
      if (st.sub_val == 1) {
        handle_scroll_use(g_character, true);
      } else {
        handle_scroll_use(g_character, false);
      }
      return;
    }
    if (itm_id.starts_with(u"0221")) {
      if (st.sub_val == 1) {
        handle_morph_use(g_character, true);
      } else {
        handle_morph_use(g_character, false);
      }
      return;
    }
  }
}

void ServerCharacterInstance::handle_s_state(
    GameCharacter &g_character,
    const std::vector<std::unique_ptr<fbs::StateT>> &v) {
  for (auto &st : v) {
    switch (st->state) {
    case StateEnum_BUFF_SKILL: {
      auto &skis = SkillGameInstance::skis();
      auto ski_id2 = std::to_string(st->val);
      auto ski_id3 = std::u16string{ski_id2.begin(), ski_id2.end()};
      if (ski_id3.length() < 7) {
        ski_id3.insert(0, 7 - ski_id3.length(), u'0');
      }
      skis.at(ski_id3).state(&g_character, st->sub_val);
      break;
    }
    case StateEnum_BUFF_ITEM: {
      handle_buff_item(g_character, *st);
      break;
    }
    case StateEnum_ITEM_USE: {
      handle_item_use(g_character, *st);
      break;
    }
    default: {
      break;
    }
    }
  }
}

void ServerCharacterInstance::handle_server_state(uint64_t client_id,
                                                    ServerCharacterStateT &r) {
  if (CharacterGameInstance::others.contains(r.client_id)) {
    auto &c =
        CharacterGameInstance::others.at(r.client_id).player_t.character;
    for (const auto &st : r.payload) {
      handle_save_state(*st, *c);
    }
    handle_s_state(CharacterGameInstance::others.at(r.client_id).g_character,
                   r.payload);
  }
}

void ServerCharacterInstance::handle_save_state(const StateT &st,
                                                  CharacterT &c) {
  for (auto it = c.states.begin(); it != c.states.end();) {
    auto &ist = *it;
    if (ist->state == st.state) {
      if (st.state == StateEnum_HP || st.state == fbs::StateEnum_MAX_HP) {
        it = c.states.erase(it);
        continue;
      }
      if (st.val == ist->val) {
        it = c.states.erase(it);
        continue;
      }
    }
    it++;
  }
  switch (st.state) {
  case StateEnum_ITEM_USE: {
    break;
  }
  case StateEnum_BUFF_SKILL: {
    if (st.sub_val != 0) {
      c.states.push_back(std::make_unique<StateT>(st));
    }
    break;
  }
  default: {
    c.states.push_back(std::make_unique<StateT>(st));
    break;
  }
  }
}

void ServerCharacterInstance::handle_lv_up(GameCharacter &g_character) {
  GameEffect e = {
      .id = u"BasicEff.img/LevelUp",
      .index = 0,
      .time = 0,
      .delay = 0,
      .lv = GameEffect::EffectSubType::effect,
      .type = GameEffect::EffectType::effect,
      .pos = SDL_FPoint{0, 0},
      .z = false,
  };
  g_character.effect.emplace_back(e);
  AudioGameInstance::load_audio(u"Game.img/LevelUp", 0);
}

void ServerCharacterInstance::handle_lv_up(uint64_t client_id,
                                             ClientCharacterLvUpT &r) {
  if (ServerClientInstance::clients.contains(client_id)) {
    auto map_id = r.map_id;
    auto scenes = ServerSceneInstance::scenes[map_id].clients;
    scenes.erase(client_id);
    ServerCharacterLvUpT t;
    t.client_id = client_id;
    for (auto c : scenes) {
      ServerResponse::send_to_client(c, t);
    }
  }
}