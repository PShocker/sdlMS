#include "ball_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_foothold.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include <cmath>
#include <cstdint>
#include <flat_map>
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

  // 1. 直接构造BallT，避免unique_ptr的单独分配
  auto ball = std::make_unique<BallT>(BallT{
      .x1 = pos.x,
      .y1 = pos.y,
      .x2 = goal.x, // 默认值
      .y2 = goal.y, // 默认值
      .speed = (float)speed,
      .mob_index = UINT32_MAX,
      .delay = delay,
      .page = (uint8_t)page,
  });

  const auto &fhs = foothold_game_instance::data;
  auto hspeed = (goal.x > pos.x) ? speed : -speed;

  // 2. 提取目标计算逻辑为lambda，减少重复
  auto calculate_target = [&](SDL_FPoint target) -> SDL_FPoint {
    auto ins = physic::fall_intersect_pos(pos, target, fhs);

    SDL_FPoint best_point = target;
    float min_dx = std::numeric_limits<float>::max();
    const float pos_x = pos.x; // 缓存pos.x

    for (const auto &[k, v] : ins) {
      bool check = false;
      // 预计算dx避免重复
      const float dx = std::abs(pos_x - v.pos.x);
      if (dx >= min_dx)
        continue; // 提前剪枝

      if (!v.fh.k.has_value()) {
        check = physic::fall_collide_wall(hspeed, v.fh, fhs);
      } else if (v.fh.k == 0) {
        check = (pos.y < target.y); // 禁止从上到下
      } else {
        // 禁止同向
        const bool moving_right = v.fh.x1 < v.fh.x2;
        const bool target_right = pos.x < target.x;
        check =
            (moving_right && target_right) || (!moving_right && !target_right);
      }
      if (check) {
        min_dx = dx;
        best_point = {v.pos.x, v.pos.y};
      }
    }
    return best_point;
  };

  // 3. 主逻辑流
  if (!cm.data.empty()) {
    const auto &mob = cm.data[0].mob;
    auto mob_r = mob_logic_system::load_rect(mob);

    if (mob_r.has_value()) {
      auto closest_pos = closest_point_on_rect(pos, mob_r.value());
      auto target = calculate_target(closest_pos);

      // 检查是否命中墙面
      if (target.x != closest_pos.x || target.y != closest_pos.y) {
        // 命中墙面
        cm.data.clear();
        ball->x2 = target.x;
        ball->y2 = target.y;
      } else {
        // 命中怪物
        ball->mob_index = mob.index;
        ball->x2 = closest_pos.x - mob.pos.x;
        ball->y2 = closest_pos.y - mob.pos.y;

        // 更新cm数据
        cm.data[0].x = ball->x2;
        cm.data[0].y = ball->y2;
      }
    } else {
      // load_rect失败，回退到goal
      auto target = calculate_target(goal);
      ball->x2 = target.x;
      ball->y2 = target.y;
    }
  } else {
    // 无cm.data
    auto target = calculate_target(goal);
    ball->x2 = target.x;
    ball->y2 = target.y;
  }

  // 4. 构造payload
  ccb.payload = std::make_unique<CharacterBallT>();
  ccb.payload->path.assign(path.begin(), path.end());
  ccb.payload->ball = std::move(ball);

  return ccb;
}

uint64_t ball_game_instance::load_ball_time(ClientCharacterBallT &cct) {
  const auto &b = cct.payload->ball;

  // 一次查找，避免 contains + at 的双重开销
  auto it = mob_game_instance::data.find(b->mob_index);
  if (it == mob_game_instance::data.end()) {
    return 0;
  }

  const auto &mob_pos = it->second.mob.pos;

  // 起点 = b->x1/y1，终点 = b->x2/y2 + mob 偏移
  const float dx = (b->x2 + mob_pos.x) - b->x1;
  const float dy = (b->y2 + mob_pos.y) - b->y1;

  const float length = sqrt(dx * dx + dy * dy);

  // 防止除零；speed 为 0 时按 0 处理（或按业务定义）
  if (b->speed <= 0.0f) {
    return 0;
  }

  // 统一转 double 计算，最后四舍五入，避免截断误差
  const double dt = (static_cast<double>(length) * 1000.0) / b->speed;
  return static_cast<uint64_t>(dt + 0.5) + b->delay;
}

std::u16string ball_game_instance::load_ball_path(const std::u16string &id) {
  std::u16string path;
  // 硬编码判断无影箭
  auto &ski = skill_game_instance::ski;
  auto it = std::ranges::find_if(
      ski, [](const game_skill &s) { return s.id == u"3101003"; });
  if (it != ski.end()) {
    path = u"310.img/skill/3101003/ball";
  } else {
    auto sub_id = id.substr(0, 4) + u".img";
    path = u"Consume/" + sub_id + u"/" + id + u"/bullet";
  }
  return path;
}

std::u16string
ball_game_instance::load_cash_ball_path(const std::u16string &id) {
  std::u16string path;
  if (!id.empty()) {
    auto sub_id = id.substr(0, 4) + u".img";
    path = u"Cash/" + sub_id + u"/" + id + u"/bullet";
  }
  return path;
}

std::u16string
ball_game_instance::load_cash_ball_effect(const std::u16string &id) {
  std::u16string effect;
  if (!id.empty()) {
    auto sub_id = id.substr(0, 4) + u".img";
    effect = u"Cash/" + sub_id + u"/" + id + u"/hit";
  }
  return effect;
}