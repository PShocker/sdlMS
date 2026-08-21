#include "server_character_instance.h"
#include "server_client_instance.h"
#include "server_party_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_chat.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/chat_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_buff_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include <cstdlib>
#include <flat_set>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>

using namespace fbs;

void server_character_instance::handle_mv(uint64_t client_id,
                                          ClientCharacterMvT &m) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  auto &client = server_client_instance::clients.at(client_id);
  auto &character = client.player_t.character;

  character->state->x = m.payload->x2;
  character->state->y = m.payload->y2;
  character->state->page = m.payload->page;

  const auto client_map_id = m.map_id;
  auto clients = server_scene_instance::scenes[client_map_id].clients;
  // 移除当前客户端，只发给其他客户端
  clients.erase(client_id);
  ServerCharacterMvT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    server_response::send_to_client(c, t);
  }
}

void server_character_instance::handle_flip(uint64_t client_id,
                                            ClientCharacterFlipT &m) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  auto &client = server_client_instance::clients.at(client_id);
  auto &character = client.player_t.character;

  character->state->flip = m.payload->flip;

  const auto client_map_id = m.map_id;
  auto clients = server_scene_instance::scenes[client_map_id].clients;
  clients.erase(client_id);
  ServerCharacterFlipT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    server_response::send_to_client(c, t);
  }
}

void server_character_instance::handle_action(uint64_t client_id,
                                              ClientCharacterActionT &m) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  auto &client = server_client_instance::clients.at(client_id);
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
  auto clients = server_scene_instance::scenes[client_map_id].clients;
  clients.erase(client_id);
  ServerCharacterActionT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    server_response::send_to_client(c, t);
  }
}

void server_character_instance::handle_die(uint64_t client_id,
                                           ClientCharacterDieT &m) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  auto &client = server_client_instance::clients.at(client_id);
  auto &character = client.player_t.character;

  character->state->x = m.payload->x;
  character->state->y = m.payload->y;
  character->state->action = "dead";
  character->state->action_index = 0;
  character->state->action_animate = true;

  const auto client_map_id = m.map_id;
  auto clients = server_scene_instance::scenes[client_map_id].clients;
  clients.erase(client_id);
  ServerCharacterDieT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    server_response::send_to_client(c, t);
  }
}

void server_character_instance::handle_fc(uint64_t client_id,
                                          ClientCharacterFcT &m) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  auto &client = server_client_instance::clients.at(client_id);
  auto &character = client.player_t.character;

  character->face->face_action = m.payload->face_action;

  const auto client_map_id = m.map_id;
  auto clients = server_scene_instance::scenes[client_map_id].clients;
  clients.erase(client_id);
  ServerCharacterFcT t;
  t.client_id = client_id;
  t.payload = std::move(m.payload);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    server_response::send_to_client(c, t);
  }
}

void server_character_instance::handle_server_mv(uint64_t client_id,
                                                 ServerCharacterMvT &m) {
  if (!character_game_instance::others.contains(m.client_id)) {
    return;
  }
  auto &mvs = character_game_instance::others[m.client_id].mvs;
  mvs.push_back(*m.payload);
}

void server_character_instance::handle_server_flip(uint64_t client_id,
                                                   ServerCharacterFlipT &m) {
  if (!character_game_instance::others.contains(m.client_id)) {
    return;
  }
  auto &c = character_game_instance::others[m.client_id].g_character;
  c.flip = m.payload->flip;
}

void server_character_instance::handle_server_action(
    uint64_t client_id, ServerCharacterActionT &m) {
  if (!character_game_instance::others.contains(m.client_id)) {
    return;
  }
  auto &c = character_game_instance::others[m.client_id].g_character;
  const auto &a = m.payload;
  c.action = std::u16string{a->action.begin(), a->action.end()};
  c.action_index = a->action_index;
  c.action_animate = a->action_animate;
  c.action_time = 0;
  auto action = character_logic_system::load_action_type(c);
  switch (action) {
  case character_logic_system::action_enum::attack: {
    character_logic_system::load_sfx(c);
    break;
  }
  case character_logic_system::action_enum::dead: {
    audio_game_instance::load_audio(u"Game.img/Tombstone", 0);
    break;
  }
  default: {
    break;
  }
  }
}

void server_character_instance::handle_server_die(uint64_t client_id,
                                                  ServerCharacterDieT &m) {
  if (!character_game_instance::others.contains(m.client_id)) {
    return;
  }
  auto &c = character_game_instance::others[m.client_id].g_character;
  const auto &d = m.payload;
  c.pos.x = d->x;
  c.pos.y = d->y;
  c.action = u"dead";
  c.action_animate = true;
  c.action_index = 0;
  c.action_time = 0;
  game_tomb t{
      .ani_type = u"fall",
      .ani_index = 0,
      .ani_time = 0,
      .pos = {d->x, d->y - 300},
      .b = c.pos,
  };
  c.tomb = t;
}

void server_character_instance::handle_server_fc(uint64_t client_id,
                                                 ServerCharacterFcT &m) {
  if (!character_game_instance::others.contains(m.client_id)) {
    return;
  }
  const auto &f = m.payload;
  auto &c = character_game_instance::others[m.client_id].g_character;
  c.face.action = std::u16string{f->face_action.begin(), f->face_action.end()};
  c.face.index = 0;
  c.face.time = 0;
  if (c.face.action == u"default") {
    c.face.destroy = window::dt_now + 4000;
  } else {
    c.face.destroy = UINT64_MAX;
  }
}

void server_character_instance::handle_skill(uint64_t client_id,
                                             ClientCharacterSkillT &r) {
  if (server_client_instance::clients.contains(client_id)) {
    auto map_id = server_client_instance::clients.at(client_id).map_id;
    auto scenes = server_scene_instance::scenes[map_id].clients;
    scenes.erase(client_id);
    ServerCharacterSkillT t;
    t.client_id = client_id;
    t.ski_id = r.ski_id;
    t.ski_lv = r.ski_lv;
    t.payload = std::move(r.payload);
    for (auto c : scenes) {
      server_response::send_to_client(c, t);
    }
  }
}

void server_character_instance::handle_attack(uint64_t client_id,
                                              ClientCharacterAttackT &r) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  auto map_id = server_client_instance::clients.at(client_id).map_id;
  auto &mobs = server_scene_instance::scenes.at(map_id).mobs;
  for (const auto &a : r.payload) {
    auto &mob = mobs.at(a->mob_index);
    mob_hit mh;
    mh.left = a->left;
    mh.hit_id = client_id;
    mh.hit_time = a->attack->delay;
    mh.hit_num = a->attack->num;
    mob.hits.emplace(mh.hit_time, mh);
  }
  // 转发
  auto clients = server_scene_instance::scenes.at(map_id).clients;
  clients.erase(client_id);
  ServerCharacterAttackT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    server_response::send_to_client(c, t);
  }
  t.client_id = 0;
  server_response::send_to_client(client_id, t);
}

void server_character_instance::handle_chat(uint64_t client_id,
                                            ClientCharacterChatT &r) {
  // 转发
  auto client_name =
      server_client_instance::clients.at(client_id).player_t.character->name;
  auto clients = server_scene_instance::scenes.at(r.map_id).clients;
  clients.erase(client_id);
  ServerCharacterChatT t;
  t.name = client_name;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    server_response::send_to_client(c, t);
  }
  t.client_id = 0;
  server_response::send_to_client(client_id, t);
}

void server_character_instance::handle_server_chat(uint64_t client_id,
                                                   ServerCharacterChatT &r) {
  game_character *g_character = nullptr;
  if (r.client_id == 0) {
    g_character = &character_game_instance::self;
  } else if (character_game_instance::others.contains(r.client_id)) {
    g_character = &character_game_instance::others[r.client_id].g_character;
  }
  if (g_character) {
    game_chatballoon c;
    c.w = 100;
    c.destroy = window::dt_now + 5000;
    c.text = {r.payload->payload.begin(), r.payload->payload.end()};
    c.size = 13;
    c.path = u"0";
    g_character->chatballoon = c;
  }

  chat_game_instance::chats.push_back({
      .type = (game_chat_enum)r.payload->type,
      .owner = {r.name.begin(), r.name.end()},
      .text = {r.payload->payload.begin(), r.payload->payload.end()},
  });
}

void server_character_instance::handle_character(uint64_t client_id,
                                                 ClientCharacterT &r) {
  if (server_client_instance::clients.contains(client_id)) {
    auto map_id = r.map_id;
    auto scenes = server_scene_instance::scenes[map_id].clients;
    scenes.erase(client_id);
    ServerCharacterT t;
    t.client_id = client_id;
    t.payload = std::move(r.payload);
    for (auto c : scenes) {
      server_response::send_to_client(c, t);
    }
    // save
    server_client_instance::clients[client_id].player_t.character =
        std::move(t.payload);
  }
}

void server_character_instance::handle_server_atk(uint64_t client_id,
                                                  ServerCharacterAttackT &r) {
  auto &mobs = mob_game_instance::data;
  std::flat_multiset<uint32_t> mob_hit;
  auto &v = r.payload;
  for (uint32_t i = 0; i < v.size(); i++) {
    auto &ct = v[i];
    auto &mob = mobs[ct->mob_index].mob;
    if (!ct->effect.empty()) {
      game_effect e = {
          .id = {ct->effect.begin(), ct->effect.end()},
          .index = 0,
          .time = 0,
          .delay = ct->attack->delay,
          .type = game_effect::effect_type::effect,
          .pos = SDL_FPoint{ct->attack->x, ct->attack->y},
          .z = false,
      };
      if (wz_resource::item->find(e.id)) {
        e.lvl = 2;
      }
      mob.effect.push_back(e);
    }
    // 伤害数字
    damage_data data = {
        .num = ct->attack->num,
        .type = damage_data::r,
    };
    game_effect d = {
        .id = u"",
        .index = (uint32_t)mob_hit.count(mob.index),
        .time = mob.index,
        .delay = ct->attack->delay,
        .type = game_effect::effect_type::damage,
        .pos = SDL_FPoint{ct->attack->x, ct->attack->y - 10},
        .z = false,
        .flip = false,
        .data = data,
    };
    mob_hit.insert(mob.index);
    effect_game_instance::data[7].emplace_back(d);
  }
}

void server_character_instance::handle_ski(
    uint32_t ski_id, uint8_t ski_lv,
    const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
    game_character &g_character) {
  auto ski_id2 = std::to_string(ski_id);
  auto ski_id3 = std::u16string{ski_id2.begin(), ski_id2.end()};

  if (ski_id3.length() < 7) {
    ski_id3.insert(0, 7 - ski_id3.length(), u'0');
  }

  g_character.skill = ski_id3;
  auto skis = skill_game_instance::skis();

  if (skis.at(ski_id3).effect) {
    game_effect e = {
        .id = ski_id3,
        .index = 0,
        .time = 0,
        .delay = window::dt_now,
        .type = game_effect::effect_type::skill_custom,
        .pos = std::nullopt,
        .z = false,
        .data = &g_character,
    };
    auto &eff = g_character.effect;
    std::erase_if(eff, [&](const auto &ef) { return ef.id == e.id; });
    eff.push_back(e);
  } else {
    game_effect e = {
        .id = ski_id3,
        .index = 0,
        .time = 0,
        .delay = 0,
        .type = game_effect::effect_type::skill_use,
        .pos = std::nullopt,
        .z = false,
        .data = &g_character,
    };
    g_character.effect.push_back(e);
  }

  audio_game_instance::load_audio(u"Skill.img/" + ski_id3 + u"/Use", 0);

  auto &mob = mob_game_instance::data;
  for (const auto &s : v) {
    game_effect e2 = {
        .id = ski_id3,
        .index = 0,
        .time = 0,
        .delay = s->delay,
        .lvl = ski_lv,
        .type = game_effect::effect_type::skill_hit,
        .pos = SDL_FPoint{s->x, s->y},
        .z = false,
    };
    mob[s->mob].mob.effect.push_back(e2);
  }
}

void server_character_instance::handle_server_ski(uint64_t client_id,
                                                  ServerCharacterSkillT &r) {
  if (character_game_instance::others.contains(r.client_id)) {
    auto &g_character =
        character_game_instance::others.at(r.client_id).g_character;
    handle_ski(r.ski_id, r.ski_lv, r.payload, g_character);
  }
}

void server_character_instance::load_g_character(
    game_character &g_character, const std::unique_ptr<CharacterT> &c) {
  const auto &appearance = c->appearance;

  std::string tmp = std::format("{:08d}", appearance->body);
  character_game_instance::add_body(g_character, {tmp.begin(), tmp.end()});

  tmp = std::format("{:08d}", appearance->head);
  character_game_instance::add_head(g_character, {tmp.begin(), tmp.end()});

  tmp = std::format("{:08d}", appearance->face);
  character_game_instance::add_face(g_character, {tmp.begin(), tmp.end()});

  tmp = std::format("{:08d}", appearance->hair);
  character_game_instance::add_hair(g_character, {tmp.begin(), tmp.end()});

  tmp = appearance->ear;
  character_game_instance::add_ear(g_character, {tmp.begin(), tmp.end()});

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
    game_equip_item e;
    e.id = {tmp.begin(), tmp.end()};
    for (const auto &scroll : equip->scroll) {
      tmp = std::format("{:08d}", scroll->scroll_id);
      e.scroll.push_back({{tmp.begin(), tmp.end()}, scroll->success});
    }
    equip_game_instance::add_equip(e, g_character, -1);
  }
  for (auto &deco : c->decos) {
    tmp = std::format("{:08d}", deco->deco_id);
    game_deco_item e;
    e.id = {tmp.begin(), tmp.end()};
    equip_game_instance::add_equip_deco(e, g_character, -1);
  }
}

game_character server_character_instance::load_g_character(
    const std::unique_ptr<CharacterT> &c) {
  game_character g_character;
  load_g_character(g_character, c);
  return g_character;
}

CharacterT server_character_instance::load_charactert(const game_character &g) {
  fbs::CharacterT c;

  c.appearance = std::make_unique<fbs::CharacterAppearanceT>();
  c.state = std::make_unique<fbs::LifeStateT>();
  c.face = std::make_unique<fbs::FaceT>();

  const auto load_equip = [&c](const std::optional<game_equip_item> &eqp) {
    if (eqp.has_value()) {
      auto id = eqp->id;
      EquipT et;
      et.equip_id = std::stoi(std::string{id.begin(), id.end()});
      for (const auto &scroll : eqp->scroll) {
        EquipScrollT est;
        est.scroll_id =
            std::stoi(std::string{scroll.id.begin(), scroll.id.end()});
        est.success = scroll.success;
        et.scroll.push_back(std::make_unique<EquipScrollT>(est));
      }
      c.equips.push_back(std::make_unique<EquipT>(et));
    }
  };

  load_equip(g.accessory);
  load_equip(g.cap);
  load_equip(g.cape);
  load_equip(g.glove);
  load_equip(g.coat);
  load_equip(g.longcoat);
  load_equip(g.pant);
  load_equip(g.shield);
  load_equip(g.weapon);
  load_equip(g.shoes);

  c.appearance->head = std::stoi(std::string{g.head.begin(), g.head.end()});
  c.appearance->body = std::stoi(std::string{g.body.begin(), g.body.end()});

  c.appearance->hair = std::stoi(std::string{g.hair.begin(), g.hair.end()});

  c.appearance->face =
      std::stoi(std::string{g.face.id.begin(), g.face.id.end()});

  c.appearance->ear = std::string{g.ear.begin(), g.ear.end()};

  if (g.coat_deco.has_value()) {
    auto id = g.coat_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.cap_deco.has_value()) {
    auto id = g.cap_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.pant_deco.has_value()) {
    auto id = g.pant_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.shoes_deco.has_value()) {
    auto id = g.shoes_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.shield_deco.has_value()) {
    auto id = g.shield_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.cape_deco.has_value()) {
    auto id = g.cape_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.accessory_deco.has_value()) {
    auto id = g.accessory_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.glove_deco.has_value()) {
    auto id = g.glove_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.longcoat_deco.has_value()) {
    auto id = g.longcoat_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.weapon_deco.has_value()) {
    auto id = g.weapon_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.ring0_deco.has_value()) {
    auto id = g.ring0_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }
  if (g.ring1_deco.has_value()) {
    auto id = g.ring1_deco->id;
    DecoT dt;
    dt.deco_id = std::stoi(std::string{id.begin(), id.end()});
    c.decos.push_back(std::make_unique<DecoT>(dt));
  }

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

void server_character_instance::handle_server_playert(
    const std::unique_ptr<fbs::PlayerT> &c) {
  auto g_character = load_g_character(c->character);

  if (g_character.action == u"dead") {
    game_tomb t{
        .ani_type = u"land",
        .ani_index = 0,
        .ani_time = 0,
        .pos = {g_character.pos.x, g_character.pos.y},
        .b = g_character.pos,
    };
    g_character.tomb = t;
  }
  character_other_data cod;
  cod.g_character = g_character;
  cod.player_t = *c;

  auto it =
      character_game_instance::others.emplace(c->client_id, std::move(cod));
  auto &character = it.first->second.g_character;
  server_character_instance::handle_s_state(character, c->character->states);
}

void server_character_instance::handle_state(uint64_t client_id,
                                             ClientCharacterStateT &r) {
  if (server_client_instance::clients.contains(client_id)) {
    auto &c = server_client_instance::clients.at(client_id).player_t.character;
    for (const auto &st : r.payload) {
      save_character_state(*st, *c);
    }
    auto scenes = server_scene_instance::scenes[r.map_id].clients;
    scenes.erase(client_id);
    ServerCharacterStateT t;
    t.client_id = client_id;
    t.payload = std::move(r.payload);
    for (auto c : scenes) {
      server_response::send_to_client(c, t);
    }
  }
}

void server_character_instance::handle_buff_item(game_character &g_character,
                                                 const StateT &st) {
  auto tmp = std::format("{:08d}", st.val);
  std::u16string itm_id{tmp.begin(), tmp.end()};
  auto item_type = item_game_instance::load_item_type(itm_id);
  if (item_type == u"Install") {
    if (itm_id.starts_with(u"0301")) {
      if (st.sub_val == 1) {
        character_logic_system::run_sit_chair(g_character, itm_id);
      } else {
        character_logic_system::run_unsit_chair(g_character);
      }
    }
  } else if (item_type == u"Consume") {
    if (itm_id.starts_with(u"0221")) {
      item_buff_game_instance::use_morph(itm_id, g_character);
    }
  }
  return;
}

void server_character_instance::handle_scroll_use(game_character &g_character,
                                                  bool success) {
  if (success) {
    game_effect e = {
        .id = u"BasicEff.img/Enchant/Success",
        .index = 0,
        .time = 0,
        .delay = 0,
        .lvl = 1,
        .type = game_effect::effect_type::effect,
        .pos = SDL_FPoint{0, 0},
        .z = false,
    };
    g_character.effect.emplace_back(e);
  } else {
    game_effect e = {
        .id = u"BasicEff.img/Enchant/Failure",
        .index = 0,
        .time = 0,
        .delay = 0,
        .lvl = 1,
        .type = game_effect::effect_type::effect,
        .pos = SDL_FPoint{0, 0},
        .z = false,
    };
    g_character.effect.emplace_back(e);
  }
}

void server_character_instance::handle_item_use(game_character &g_character,
                                                const StateT &st) {
  auto tmp = std::format("{:08d}", st.val);
  std::u16string itm_id{tmp.begin(), tmp.end()};
  auto item_type = item_game_instance::load_item_type(itm_id);
  if (item_type == u"Consume") {
    if (itm_id.starts_with(u"0204")) {
      if (st.sub_val == 1) {
        handle_scroll_use(g_character, true);
      } else {
        handle_scroll_use(g_character, false);
      }
    }
  }
}

void server_character_instance::handle_s_state(
    game_character &g_character,
    const std::vector<std::unique_ptr<fbs::StateT>> &v) {
  for (auto &st : v) {
    switch (st->state) {
    case StateEnum_BUFF_SKILL:
    case StateEnum_BUFF_ABNORMAL: {
      auto &skis = skill_game_instance::skis();
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

void server_character_instance::handle_server_state(uint64_t client_id,
                                                    ServerCharacterStateT &r) {
  if (character_game_instance::others.contains(r.client_id)) {
    auto &c =
        character_game_instance::others.at(r.client_id).player_t.character;
    for (const auto &st : r.payload) {
      save_character_state(*st, *c);
    }
    handle_s_state(character_game_instance::others.at(r.client_id).g_character,
                   r.payload);
  }
}

void server_character_instance::remove_character_state(StateT s,
                                                       CharacterT &c) {
  std::erase_if(c.states, [&](const auto &st) {
    switch (s.state) {
    case StateEnum_HP:
    case StateEnum_MAX_HP: {
      return st->state == s.state;
      break;
    }
    case StateEnum_BUFF_SKILL:
    case StateEnum_BUFF_ITEM:
    case StateEnum_BUFF_ABNORMAL: {
      if (s.sub_val == 0 && s.val == st->val) {
        return true;
      }
      break;
    }
    default: {
      break;
    }
    }
    return false;
  });
}

void server_character_instance::save_character_state(const StateT &st,
                                                     CharacterT &c) {
  remove_character_state(st, c);
  switch (st.state) {
  case StateEnum_ITEM_USE: {
    break;
  }
  default: {
    c.states.push_back(std::make_unique<StateT>(st));
    break;
  }
  }
}

void server_character_instance::handle_lv_up(game_character &g_character) {
  game_effect e = {
      .id = u"BasicEff.img/LevelUp",
      .index = 0,
      .time = 0,
      .delay = 0,
      .lvl = 1,
      .type = game_effect::effect_type::effect,
      .pos = SDL_FPoint{0, 0},
      .z = false,
  };
  g_character.effect.emplace_back(e);
}

void server_character_instance::handle_lv_up(uint64_t client_id,
                                             ClientCharacterLvUpT &r) {
  if (server_client_instance::clients.contains(client_id)) {
    auto map_id = r.map_id;
    auto scenes = server_scene_instance::scenes[map_id].clients;
    scenes.erase(client_id);
    ServerCharacterLvUpT t;
    t.client_id = client_id;
    for (auto c : scenes) {
      server_response::send_to_client(c, t);
    }
  }
}