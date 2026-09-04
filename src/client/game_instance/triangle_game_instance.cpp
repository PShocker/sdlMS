#include "triangle_game_instance.h"
#include "src/client/game/game_triangle.h"

bool triangle_game_instance::rect_ins_tri(SDL_FRect &r, game_triangle &t) {
  // 1. 获取矩形4个顶点
  std::array<SDL_FPoint, 4> rect_pts = {
      {{r.x, r.y}, {r.x + r.w, r.y}, {r.x, r.y + r.h}, {r.x + r.w, r.y + r.h}}};

  // 2. 获取三角形3个顶点
  auto &[v1, v2, v3] = t.point;
  std::array<SDL_FPoint, 3> tri_pts = {{v1, v2, v3}};

  // 3. 投影函数：将多边形投影到指定轴上
  auto project = [](const auto &pts, const SDL_FPoint &axis, float &min,
                    float &max) {
    // 计算第一个顶点的投影值
    min = max = pts[0].x * axis.x + pts[0].y * axis.y;

    // 计算其余顶点的投影值，更新最小值和最大值
    for (size_t i = 1; i < pts.size(); i++) {
      float proj = pts[i].x * axis.x + pts[i].y * axis.y;
      if (proj < min)
        min = proj;
      if (proj > max)
        max = proj;
    }
  };

  // 4. 定义所有需要检测的分离轴
  //    矩形：水平轴(1,0) 和 垂直轴(0,1)
  //    三角形：三条边的法线方向
  std::array<SDL_FPoint, 5> axes = {{
      {1.0f, 0.0f},                  // 矩形水平轴
      {0.0f, 1.0f},                  // 矩形垂直轴
      {-(v2.y - v1.y), v2.x - v1.x}, // 三角形边 v1->v2 的法线
      {-(v3.y - v2.y), v3.x - v2.x}, // 三角形边 v2->v3 的法线
      {-(v1.y - v3.y), v1.x - v3.x}  // 三角形边 v3->v1 的法线
  }};

  // 5. 对每个轴进行投影测试
  for (const auto &axis : axes) {
    // 跳过零向量（防止退化三角形）
    if (axis.x == 0.0f && axis.y == 0.0f)
      continue;

    float min1, max1, min2, max2;

    // 矩形在轴上的投影
    project(rect_pts, axis, min1, max1);

    // 三角形在轴上的投影
    project(tri_pts, axis, min2, max2);

    // 如果投影区间不重叠，说明找到了分离轴，两个图形不相交
    if (max1 < min2 || max2 < min1) {
      return false;
    }
  }

  // 所有轴都重叠，两个图形相交
  return true;
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