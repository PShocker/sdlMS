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

  // 使用初始化列表或直接构造，避免多次unique_ptr分配
  auto ball = std::make_unique<BallT>();
  ball->page = page;
  ball->speed = speed;
  ball->delay = delay;
  ball->x1 = pos.x;
  ball->y1 = pos.y;

  // 处理目标位置
  const auto &fhs = foothold_game_instance::data;
  SDL_FPoint target_pos = goal; // 默认目标

  if (!cm.data.empty()) {
    const auto &mob = cm.data[0].mob;
    auto mob_r = mob_logic_system::load_rect(mob);

    if (mob_r.has_value()) {
      auto closest_pos = closest_point_on_rect(pos, mob_r.value());
      auto ins = physic::fall_intersect_pos(pos, closest_pos, fhs);

      if (!ins.empty()) {
        // 命中墙面
        cm.data.clear(); // 更清晰的清空方式
        // 找到x最近的点
        
        target_pos = {ins.begin()->second.pos.x, ins.begin()->second.pos.y};
        ball->mob = false;
      } else {
        // 命中怪物
        ball->mob = true;
        ball->mob_index = mob.index;
        target_pos = {closest_pos.x - mob.pos.x, closest_pos.y - mob.pos.y};
        // 更新cm数据
        cm.data[0].x = target_pos.x;
        cm.data[0].y = target_pos.y;
      }
    } else {
      // load_rect失败时的回退处理
      ball->mob = false;
      // 使用goal作为目标
      auto ins = physic::fall_intersect_pos(pos, goal, fhs);
      target_pos = ins.empty() ? goal
                               : SDL_FPoint{
                                     ins.begin()->second.pos.x,
                                     ins.begin()->second.pos.y,
                                 };
    }
  } else {
    // 无cm.data时
    auto ins = physic::fall_intersect_pos(pos, goal, fhs);
    // if (fall_collide_wall(hspeed, fh, fhs)) {
    // }
    target_pos = ins.empty() ? goal
                             : SDL_FPoint{
                                   ins.begin()->second.pos.x,
                                   ins.begin()->second.pos.y,
                               };
    ball->mob = false;
  }

  // 设置目标位置
  ball->x2 = target_pos.x;
  ball->y2 = target_pos.y;

  // 构造payload
  ccb.payload = std::make_unique<CharacterBallT>();
  ccb.payload->path.assign(path.begin(), path.end());
  ccb.payload->ball = std::move(ball);

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