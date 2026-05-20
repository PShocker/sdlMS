#include "effect_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <string>

void effect_game_instance::reset() {
  effect = {};
  effect2 = {};
  data = {};
}

void effect_game_instance::load_character_attack(
    const std::vector<std::unique_ptr<fbs::CharacterAttackT>> &v,
    game_character &g_character) {
  for (uint32_t i = 0; i < v.size(); i++) {
    auto &ct = v[i];
    if (ct->afterimage) {
      game_effect e = {
          .id = afterimage_game_instance::load_hit_type(g_character),
          .index = 0,
          .time = 0,
          .delay = ct->attack->delay,
          .type = game_effect::effect_type::afterimage,
          .pos = SDL_FPoint{ct->attack->x, ct->attack->y},
          .z = false,
      };
      effect2[ct->mob_index].emplace_back(e);
    }
    game_effect d = {
        .id = u"",
        .index = 0,
        .time = 0,
        .delay = ct->attack->delay,
        .type = game_effect::effect_type::damage,
        .pos = SDL_FPoint{ct->attack->x, ct->attack->y},
        .z = false,
        .data = (int32_t)ct->attack->num,
    };
    data[7].emplace_back(d);
  }
}

void effect_game_instance::load_character_skill(
    uint32_t ski_id,
    const std::vector<std::unique_ptr<fbs::CharacterSkillT>> &v,
    game_character &g_character) {
  auto ski_id2 = std::to_string(ski_id);
  auto ski_id3 = std::u16string{ski_id2.begin(), ski_id2.end()};
  for (const auto &s : v) {
    game_effect e = {
        .id = ski_id3,
        .index = 0,
        .time = 0,
        .delay = s->delay,
        .type = game_effect::effect_type::skill_hit,
        .pos = SDL_FPoint{s->x, s->y},
        .z = false,
    };
  }
}