#include "physic.h"
#include "src/client/game/game_foothold.h"
#include "src/client/window/window.h"
#include <algorithm>

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
    return false;
  }
  if (!n_fh->k.has_value()) {
    if (pos.y == std::clamp(pos.y, (float)n_fh->b - 1, (float)n_fh->b + 1) ||
        pos.y >= (float)n_fh->b + 1) {
      // 撞墙
      const auto &c_fh = fhs.at(current_fh);
      pos.x = hspeed < 0 ? c_fh.x1 + 0.1 : c_fh.x1 - 0.1;
      pos.x = std::clamp(pos.x, (float)c_fh.l, (float)c_fh.r);
      pos.y = c_fh.k.value() * pos.x + c_fh.intercept.value();
      hspeed = 0;
      return false;
    } else if (fall) {
      const auto &c_fh = fhs.at(current_fh);
      // 楼梯上掉落
      vspeed = 0;
      pos.x = hspeed < 0 ? c_fh.x1 - 0.1 : c_fh.x1 + 0.1;
      current_fh = 0;
      return false;
    } else {
      const auto &c_fh = fhs.at(current_fh);
      // 禁止掉落
      pos.x = hspeed < 0 ? c_fh.x1 + 0.1 : c_fh.x1 - 0.1;
      return false;
    }
  }
  current_fh = next_fh;
  return true;
}

bool physic::walk(SDL_FPoint &pos,float delta_time, float &hspeed, float &vspeed, float &hforce,
                  float &hspeed_min, float &hspeed_max, float friction,
                  bool fall, int32_t &current_fh, const SDL_FRect &border,
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

  // 往左走
  while (true) {
    const auto &c_fh = fhs.at(current_fh);
    if (pos.x >= c_fh.l) {
      break;
    }
    auto prev_fh = c_fh.prev;
    if (walk_fh(pos, true, prev_fh, current_fh, hspeed, vspeed, fhs) == false) {
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
    if (walk_fh(pos, true, prev_fh, current_fh, hspeed, vspeed, fhs) == false) {
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
    if (walk_fh(pos, true, next_fh, current_fh, hspeed, vspeed, fhs) == false) {
      return false;
    }
  }
  const auto &cur_fh = fhs.at(current_fh);
  pos.y = cur_fh.k.value() * pos.x + cur_fh.intercept.value();
  return true;
}