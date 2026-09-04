#pragma once

#include "src/client/game/game_character.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
#include <string>
using namespace fbs;

class server_character_instance {
private:
  static void remove_character_state(StateT s, CharacterT &c);
  static void save_character_state(const StateT &v, CharacterT &c);

public:
  static void handle_attack(uint64_t client_id, ClientCharacterAttackT &r);

  static void handle_mv(uint64_t client_id, ClientCharacterMvT &m);
  static void handle_flip(uint64_t client_id, ClientCharacterFlipT &m);
  static void handle_action(uint64_t client_id, ClientCharacterActionT &m);
  static void handle_die(uint64_t client_id, ClientCharacterDieT &m);
  static void handle_fc(uint64_t client_id, ClientCharacterFcT &m);

  static void handle_server_mv(uint64_t client_id, ServerCharacterMvT &m);
  static void handle_server_flip(uint64_t client_id, ServerCharacterFlipT &m);
  static void handle_server_action(uint64_t client_id,
                                   ServerCharacterActionT &m);
  static void handle_server_die(uint64_t client_id, ServerCharacterDieT &m);
  static void handle_server_fc(uint64_t client_id, ServerCharacterFcT &m);

  static void handle_skill(uint64_t client_id, ClientCharacterSkillT &r);
  static void handle_chat(uint64_t client_id, ClientCharacterChatT &r);
  static void handle_server_chat(uint64_t client_id, ServerCharacterChatT &r);
  static void handle_character(uint64_t client_id, ClientCharacterT &r);

  static void handle_server_atk(uint64_t client_id, ServerCharacterAttackT &r);

  static void load_g_character(game_character &g_character,
                               const std::unique_ptr<CharacterT> &c);
  static game_character load_g_character(const std::unique_ptr<CharacterT> &c);
  static CharacterT load_charactert(const game_character &g);
  static void handle_server_playert(const std::unique_ptr<fbs::PlayerT> &c);

  static void
  handle_ski(uint32_t ski_id, uint8_t ski_lv,
             const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
             game_character &g_character);
  static void handle_server_ski(uint64_t client_id, ServerCharacterSkillT &r);

  static void
  handle_s_state(game_character &g_character,
                 const std::vector<std::unique_ptr<fbs::StateT>> &v);

  static void handle_state(uint64_t client_id, ClientCharacterStateT &r);
  static void handle_server_state(uint64_t client_id, ServerCharacterStateT &r);

  static void handle_lv_up(game_character &g_character);
  static void handle_lv_up(uint64_t client_id, ClientCharacterLvUpT &r);

  static void handle_buff_item(game_character &g_character, const StateT &st);

  static void handle_morph_use(game_character &g_character, bool success);
  static void handle_scroll_use(game_character &g_character, bool success);
  static void handle_item_use(game_character &g_character, const StateT &st);
};