#include "server_character_instance.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include <cstdlib>
#include <flat_set>
#include <memory>
#include <ranges>
#include <string>
#include <utility>

using namespace fbs;

void server_character_instance::save_state(uint64_t client_id,
                                           ClientCharacterLogicT &m) {

  auto &client = server_client_instance::clients.at(client_id);
  auto &character = client.player_t.character;
  switch (m.payload.type) {
  case CharacterLogicType_Movement: {
    const auto mv = m.payload.AsMovement();
    character->state->x = mv->x2;
    character->state->y = mv->y2;
    character->state->page = mv->page;
    break;
  }
  case CharacterLogicType_Flip: {
    const auto f = m.payload.AsFlip();
    character->state->flip = f->flip;
    break;
  }
  case CharacterLogicType_Action: {
    const static std::flat_set<std::string> actions = {
        "stand1", "stand2", "alert",  "walk1", "walk2",
        "prone",  "jump",   "ladder", "rope",  "dead",
    };
    const auto a = m.payload.AsAction();
    if (actions.contains(a->action)) {
      character->state->action = a->action;
      character->state->action_index = a->action_index;
      character->state->action_animate = a->action_animate;
    }
    break;
  }
  case CharacterLogicType_Die: {
    const auto d = m.payload.AsDie();
    character->state->x = d->x;
    character->state->y = d->y;
    character->state->action = "dead";
    character->state->action_index = 0;
    character->state->action_animate = true;
    break;
  }
  case CharacterLogicType_Face: {
    const auto f = m.payload.AsFace();
    character->face->face_action = f->face_action;
    break;
  }
  default: {
    std::abort();
    break;
  }
  }
}

void server_character_instance::send_logic(uint64_t client_id,
                                           ClientCharacterLogicT &m) {
  const auto client_map_id = m.map_id;
  auto clients = server_scene_instance::scenes[client_map_id].clients;

  // 移除当前客户端，只发给其他客户端
  clients.erase(client_id);

  // 创建要发送的数据包
  ServerCharacterLogicT t; // 假设这是响应的包装结构
                           // 关键：将union设置到payload中
  CharacterLogicT t2;
  t2.client_id = client_id;
  t2.payload = m.payload;
  t.payload = std::make_unique<CharacterLogicT>(t2);
  // 广播给其他所有客户端
  for (const auto c : clients) {
    server_response::send_to_client(c, t);
  }
}

void server_character_instance::handle_logic(uint64_t client_id,
                                             ClientCharacterLogicT &m) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  send_logic(client_id, m);
  save_state(client_id, m);
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
    mob_beat mbb;
    mbb.beat_id = client_id;
    mbb.beat_start_time = a->attack->delay;
    mbb.left = a->left;
    mbb.beat_time = 300;
    mob.beats.emplace(mbb.beat_start_time, mbb);
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
    c.destory = window::dt_now + 5000;
    c.text = {r.payload->payload.begin(), r.payload->payload.end()};
    c.size = 13;
    c.path = u"0";
    g_character->chatballoon = c;
  }
  statusbar_ui_system::chats_info.push_back({
      .type = (statusbar_ui_system::chat_enum)r.payload->type,
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