#include "server_character_instance.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include <cstdlib>
#include <ranges>
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
    const auto a = m.payload.AsAction();
    character->state->action = a->action;
    character->state->action_index = a->action_index;
    character->state->action_animate = a->action_animate;
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
    server_response::character_logic_response(c, t);
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
      server_response::character_skill_response(c, t);
    }
  }
}