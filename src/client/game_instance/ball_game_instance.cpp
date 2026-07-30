#include "ball_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "equip_game_instance.h"
#include "src/client/game/game_ball.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

void ball_game_instance::reset() { data = {}; }

SDL_FPoint ball_game_instance::closest_point_on_rect(const SDL_FPoint &pos,
                                                     const SDL_FRect &rect) {
  SDL_FPoint closest;

  // X 方向夹紧（clamp）
  if (pos.x < rect.x)
    closest.x = rect.x;
  else if (pos.x > rect.x + rect.w)
    closest.x = rect.x + rect.w;
  else
    closest.x = pos.x; // 点在矩形 X 范围内

  // Y 方向夹紧（clamp）
  if (pos.y < rect.y)
    closest.y = rect.y;
  else if (pos.y > rect.y + rect.h)
    closest.y = rect.y + rect.h;
  else
    closest.y = pos.y; // 点在矩形 Y 范围内

  return closest;
}

ClientCharacterBallT ball_game_instance::create_ball_payload(
    check_mobs &cm, SDL_FPoint pos, SDL_FPoint goal, uint64_t delay, int page,
    int speed, const std::u16string &path) {
  ClientCharacterBallT ccb;
  ccb.payload = std::make_unique<CharacterBallT>();
  ccb.payload->path = {path.begin(), path.end()};
  ccb.payload->ball = std::make_unique<BallT>();
  ccb.payload->ball->page = page;
  ccb.payload->ball->speed = speed;
  ccb.payload->ball->delay = delay;
  ccb.payload->ball->x1 = pos.x;
  ccb.payload->ball->y1 = pos.y;
  if (!cm.data.empty()) {
    // wall
    const auto &mob = cm.data[0].mob;
    auto mob_r = mob_logic_system::load_rect(mob).value();
    auto closest_pos = closest_point_on_rect(pos, mob_r);
    const auto &fhs = foothold_game_instance::data;
    auto ins = physic::fall_intersect_pos(pos, closest_pos, fhs);
    if (!ins.empty()) {
      ccb.payload->ball->x2 = ins.begin()->second.pos.x;
      ccb.payload->ball->y2 = ins.begin()->second.pos.y;
      ccb.payload->ball->mob = false;
    } else {
      ccb.payload->ball->mob = true;
      ccb.payload->ball->mob_index = mob.index;
      ccb.payload->ball->x2 = closest_pos.x - mob.pos.x;
      ccb.payload->ball->y2 = closest_pos.y - mob.pos.y;
      cm.data[0].x = ccb.payload->ball->x2;
      cm.data[0].y = ccb.payload->ball->y2;
    }
  } else {
    ccb.payload->ball->x2 = goal.x;
    ccb.payload->ball->y2 = goal.y;
    ccb.payload->ball->mob = false;
  }

  return ccb;
}

uint64_t ball_game_instance::load_ball_time(ClientCharacterBallT &cct) {
  auto &b = cct.payload->ball;
  if (b->mob) {
    auto p1 = SDL_FPoint{b->x1, b->y1};
    auto p2 = SDL_FPoint{b->x2, b->y2};
    auto mob_index = b->mob_index;
    auto mob_pos = mob_game_instance::data.at(mob_index).mob.pos;
    p2.x += mob_pos.x;
    p2.y += mob_pos.y;

    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float length = sqrtf(dx * dx + dy * dy);

    uint64_t dt = (length * 1000) / (float)(b->speed);
    return dt + b->delay;
  }
  return 0;
}

ball_game_instance::ball_enum
ball_game_instance::load_ball_type(game_character &g_character) {
  auto weapon_type = equip_game_instance::load_weapon_type(g_character);
  switch (weapon_type) {
  case equip_game_instance::weapon_type::BOW: {
    return ball_enum::bow;
    break;
  }
  case equip_game_instance::weapon_type::CROSSBOW: {
    return ball_enum::arrow;
    break;
  }
  case equip_game_instance::weapon_type::CLAW: {
    return ball_enum::claw;
    break;
  }
  default: {
    break;
  }
  }
  return ball_enum::none;
}

std::u16string ball_game_instance::load_pkg_ball(int i, ball_enum type) {
  std::u16string pre;
  switch (type) {
  case ball_enum::claw: {
    pre = u"0207";
    break;
  }
  case ball_enum::bow:
  case ball_enum::arrow:
    break;

  default: {
    return u"";
  }
  }

  auto pkg = package_game_instance::data[(int)item_enum::consume];
  for (auto itm : pkg) {
    if (itm->id.empty()) {
      continue;
    }
    std::u16string sub = itm->id.substr(0, 3);
    if (pre == sub) {
      auto itm_num = package_ui_system::load_full_item_num(itm->id);
      if (itm_num >= i) {
        return itm->id;
      }
    }
  }
  return u"";
}