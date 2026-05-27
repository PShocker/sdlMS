#include "physic.h"
#include "src/client/game/game_foothold.h"
#include "src/client/window/window.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ranges>

bool physic::walk_fh(SDL_FPoint &pos, bool fall, int32_t next_fh,
                     int32_t &current_fh, float &hspeed, float &vspeed,
                     const std::flat_map<int32_t, game_foothold> &fhs) {
  const game_foothold *n_fh = nullptr;
  if (next_fh != 0) {
    n_fh = &fhs.at(next_fh);
  } else if (fall) {
    vspeed = 0;
    current_fh = 0;
    return false;
  } else {
    // 禁止掉落
    const auto &c_fh = fhs.at(current_fh);
    pos.x = hspeed < 0 ? c_fh.l + 0.1 : c_fh.r - 0.1;
    hspeed = 0;
    vspeed = 0;
    return true;
  }
  if (!n_fh->k.has_value()) {
    if (pos.y == std::clamp(pos.y, (float)n_fh->b - 1, (float)n_fh->b + 1) ||
        pos.y >= (float)n_fh->b + 1) {
      // 撞墙
      const auto &c_fh = fhs.at(current_fh);
      pos.x = std::clamp(pos.x, (float)c_fh.l, (float)c_fh.r);
      pos.x = hspeed < 0 ? pos.x + 0.1 : pos.x - 0.1;
      pos.y = c_fh.k.value() * pos.x + c_fh.intercept.value();
      hspeed = 0;
      return true;
    } else if (fall) {
      // 楼梯上掉落
      vspeed = 0;
      pos.x = hspeed < 0 ? pos.x - 0.1 : pos.x + 0.1;
      current_fh = 0;
      return false;
    } else {
      const auto &c_fh = fhs.at(current_fh);
      // 禁止掉落
      pos.x = hspeed < 0 ? pos.x - 0.1 : pos.x + 0.1;
      pos.x = std::clamp(pos.x, (float)c_fh.l, (float)c_fh.r);
      pos.y = c_fh.k.value() * pos.x + c_fh.intercept.value();
      return true;
    }
  }
  current_fh = next_fh;
  return true;
}

bool physic::walk(SDL_FPoint &pos, float delta_time, float &hspeed,
                  float &vspeed, float &hforce, float hspeed_min,
                  float hspeed_max, float friction, bool fall,
                  int32_t &current_fh, std::optional<SDL_FRect> border,
                  const std::flat_map<int32_t, game_foothold> &fhs) {
  // 判断摩擦力方向
  if (hspeed > 0) {
    hforce -= friction;
    hforce = std::max(-hspeed / delta_time, hforce);
  } else if (hspeed < 0) {
    hforce += friction;
    hforce = std::min(-hspeed / delta_time, hforce);
  }

  hspeed += delta_time * hforce;
  hspeed = std::clamp(hspeed, hspeed_min, hspeed_max);
  pos.x = hspeed * delta_time + pos.x;
  if (border.has_value()) {
    pos.x = std::clamp(pos.x, border->x, border->w);
  }

  // 往左走
  while (true) {
    const auto &c_fh = fhs.at(current_fh);
    if (pos.x >= c_fh.l) {
      break;
    }
    auto prev_fh = c_fh.prev;
    if (walk_fh(pos, fall, prev_fh, current_fh, hspeed, vspeed, fhs) == false) {
      return false;
    }
  }
  // 往左走
  while (true) {
    const auto &c_fh = fhs.at(current_fh);
    if (pos.x >= c_fh.l) {
      break;
    }
    auto prev_fh = c_fh.prev;
    if (walk_fh(pos, fall, prev_fh, current_fh, hspeed, vspeed, fhs) == false) {
      return false;
    }
  }
  // 往右走
  while (true) {
    const auto &c_fh = fhs.at(current_fh);
    if (pos.x <= c_fh.r) {
      break;
    }
    auto next_fh = c_fh.next;
    if (walk_fh(pos, fall, next_fh, current_fh, hspeed, vspeed, fhs) == false) {
      return false;
    }
  }
  const auto &cur_fh = fhs.at(current_fh);
  pos.y = cur_fh.k.value() * pos.x + cur_fh.intercept.value();
  return true;
}

std::optional<SDL_FPoint> physic::fall_intersect(const SDL_FPoint &p1,
                                                 const SDL_FPoint &p2,
                                                 const SDL_FPoint &p3,
                                                 const SDL_FPoint &p4) {
  // 预计算差值（复用）
  const float dx1 = p2.x - p1.x;
  const float dy1 = p2.y - p1.y;
  const float dx2 = p4.x - p3.x;
  const float dy2 = p4.y - p3.y;

  // 快速排斥（使用fmaxf/fminf，编译器会优化为内联）
  if (fmaxf(p1.x, p2.x) < fminf(p3.x, p4.x) ||
      fmaxf(p1.y, p2.y) < fminf(p3.y, p4.y) ||
      fmaxf(p3.x, p4.x) < fminf(p1.x, p2.x) ||
      fmaxf(p3.y, p4.y) < fminf(p1.y, p2.y)) {
    return std::nullopt;
  }

  // 跨立实验（使用fma优化）
  const float cross1 = (p1.x - p3.x) * dy2 - (p1.y - p3.y) * dx2;
  const float cross2 = (p2.x - p3.x) * dy2 - (p2.y - p3.y) * dx2;
  const float cross3 = (p3.x - p1.x) * dy1 - (p3.y - p1.y) * dx1;
  const float cross4 = (p4.x - p1.x) * dy1 - (p4.y - p1.y) * dx1;

  if (cross1 * cross2 > 0 || cross3 * cross4 > 0) {
    return std::nullopt;
  }

  // 计算交点（使用优化的公式）
  const float denominator = dx1 * dy2 - dy1 * dx2;
  if (denominator == 0)
    return std::nullopt;

  // 只用除法一次，其他都是乘加
  const float t = ((p3.x - p1.x) * dy2 - (p3.y - p1.y) * dx2) / denominator;

  // 直接返回（不需要clamp，理论上t已经在[0,1]内）
  return SDL_FPoint{p1.x + t * dx1, p1.y + t * dy1};
}

std::flat_map<float, physic::intersect_pos>
physic::fall_intersect_pos(const SDL_FPoint &p1, const SDL_FPoint &p2,
                           const std::flat_map<int32_t, game_foothold> &fhs) {
  std::flat_map<float, physic::intersect_pos> r;
  for (const auto &fh : fhs | std::views::values) {
    auto collide = fall_intersect(p1, p2, {(float)fh.x1, (float)fh.y1},
                                  {(float)fh.x2, (float)fh.y2});
    if (collide.has_value()) {
      physic::intersect_pos inter_pos;
      inter_pos.fh = fh;
      inter_pos.pos = collide.value();
      r.emplace(inter_pos.pos.y, inter_pos);
    }
  }
  return r;
}

bool physic::fall_collide_wall(
    float hspeed, game_foothold fh,
    const std::flat_map<int32_t, game_foothold> &fhs) {
  bool r = false;
  if (fh.k.has_value()) {
    return true;
  }
  if (hspeed > 0 && fh.y1 > fh.y2) {
    while (fh.prev != 0) {
      fh = fhs.at(fh.prev);
      if (fh.k.has_value()) {
        return true;
      }
    }
  } else if (hspeed < 0 && fh.y1 < fh.y2) {
    while (fh.next != 0) {
      fh = fhs.at(fh.next);
      if (fh.k.has_value()) {
        return true;
      }
    }
  }
  return r;
}

bool physic::fall(SDL_FPoint &pos, float delta_time, float &hspeed,
                  float &vspeed, float vspeed_min, float vspeed_max,
                  std::optional<SDL_FRect> border, bool fall_collide,
                  bool wall_collide, int32_t &current_fh, uint8_t &page,
                  const std::flat_map<int32_t, game_foothold> &fhs) {
  vspeed = std::clamp(vspeed, vspeed_min, vspeed_max);
  SDL_FPoint new_pos = pos;
  new_pos.x += hspeed * delta_time;
  new_pos.y += vspeed * delta_time;
  auto inter_pos = fall_intersect_pos(pos, new_pos, fhs);
  pos = new_pos;
  if (border.has_value()) {
    pos.x = std::clamp(pos.x, border->x, border->w);
    pos.y = std::clamp(pos.y, border->y, border->h);
  }
  // 下落
  if (vspeed >= 0) {
    if (fall_collide) {
      while (!inter_pos.empty()) {
        auto first = *inter_pos.begin(); // 下落取y最小
        const auto &collide_pos = first.second.pos;
        const auto &fh = first.second.fh;
        if (fh.x2 >= fh.x1) {
          // 碰撞
          if (fh.k.has_value()) {
            // 落地
            current_fh = fh.id;
            hspeed = hspeed / 2;
            vspeed = 0;
            page = fh.page;
            pos = collide_pos;
            return false;
          } else {
            // 撞墙
            if (fall_collide_wall(hspeed, fh, fhs)) {
              pos = collide_pos;
              pos.x += hspeed < 0 ? 0.1 : -0.1;
              hspeed = 0;
              return true;
            }
          }
        }
        inter_pos.erase(inter_pos.begin());
      }
    }
  } else {
    // vspeed<0
    // 只需要和天花板,墙碰撞
    while (!inter_pos.empty()) {
      auto last = *inter_pos.rbegin(); // 上升取y最大
      const auto &collide_pos = last.second.pos;
      const auto &fh = last.second.fh;
      if (!fh.k.has_value()) {
        // 撞墙
        if (fall_collide_wall(hspeed, fh, fhs)) {
          pos.x = fh.x1;
          pos.x += hspeed < 0 ? 0.1 : -0.1;
          hspeed = 0;
          return true;
        }
      } else if (fh.x2 < fh.x1 && fh.zmass == 0) {
        if (fall_collide_wall(hspeed, fh, fhs)) {
          vspeed = 0;
          pos.y = collide_pos.y;
          return true;
        }
      }
      inter_pos.erase(std::prev(inter_pos.end()));
    }
  }
  return true;
}