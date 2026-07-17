#include "triangle_game_instance.h"
#include "src/client/game/game_triangle.h"

static bool point_in_tri(const SDL_FPoint &pt, const game_triangle &tri) {
  auto &[v1, v2, v3] = tri.point;
  float area = 0.5f * (-v2.y * v3.x + v1.y * (-v2.x + v3.x) +
                       v1.x * (v2.y - v3.y) + v2.x * v3.y);
  float s =
      1 / (2 * area) *
      (v1.y * v3.x - v1.x * v3.y + (v3.y - v1.y) * pt.x + (v1.x - v3.x) * pt.y);
  float t =
      1 / (2 * area) *
      (v1.x * v2.y - v1.y * v2.x + (v1.y - v2.y) * pt.x + (v2.x - v1.x) * pt.y);
  return s >= 0 && t >= 0 && (s + t <= 1);
}

bool triangle_game_instance::rect_ins_tri(SDL_FRect &r, game_triangle &t) {
  SDL_FPoint points[4] = {
      {r.x, r.y},
      {r.x + r.w, r.y},
      {r.x, r.y + r.h},
      {r.x + r.w, r.y + r.h},
  };

  // 检查矩形的每个角点是否在三角形内
  for (const SDL_FPoint &pt : points) {
    if (point_in_tri(pt, t)) {
      return true; // 有一个点在三角形内
    }
  }
  return false; // 所有点都不在三角形内
}

game_triangle triangle_game_instance::load_tri(game_triangle t, bool flip,
                                               SDL_FPoint &pos) {
  if (flip) {
    for (auto &p : t.point) {
      p.x = pos.x - p.x;
      p.y = pos.y - p.y;
    }

  } else {
    for (auto &p : t.point) {
      p.x += pos.x;
      p.y += pos.y;
    }
  }
  return t;
}