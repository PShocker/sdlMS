#include "ball_logic_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include "wz/Property.h"

void ball_logic_system::run_animate(game_ball &b) {
  wz::Node *n = nullptr;
  if (wz_resource::skill->find(b.path)) {
    n = wz_resource::skill->find(b.path);
  } else {
    n = wz_resource::item->find(b.path);
  }
  b.ani_time += window::delta_time;
  auto index = std::to_string(b.ani_index);
  auto texture_node = n->get_child(index);
  int delay = 100;
  if (texture_node->get_child(u"delay")) {
    delay = static_cast<wz::Property<int> *>(texture_node->get_child(u"delay"))
                ->get();
  }
  if (b.ani_time >= delay) {
    b.ani_time = 0;
    b.ani_index += 1;
    b.ani_index = b.ani_index % n->children_count();
  }
  return;
}

SDL_FPoint ball_logic_system::move(SDL_FPoint p1, SDL_FPoint p2, int speed) {
  auto step = speed * window::delta_time / 1000.0f;

  float dx = p2.x - p1.x;
  float dy = p2.y - p1.y;
  float length = sqrtf(dx * dx + dy * dy);

  // 如果剩余距离 <= 步长，直接到达终点并停止
  if (length <= step) {
    return p2;
  }

  // 否则正常移动 step 距离
  float normX = dx / length;
  float normY = dy / length;

  SDL_FPoint result;
  result.x = p1.x + normX * step;
  result.y = p1.y + normY * step;

  return result;
}

bool ball_logic_system::run_move(game_ball &b) {
  if (b.mob_index.has_value()) {
    auto mob_index = b.mob_index.value();
    auto mob_pos = mob_game_instance::data.at(mob_index).mob.pos;
    auto p2 = b.goal;
    p2.x += mob_pos.x;
    p2.y += mob_pos.y;
    auto p = move(b.pos, p2, b.speed);
    b.pos = p;
    if (p.x == p2.x && p.y == p2.y) {
      // 删除ball
      return false;
    }
  } else {
    auto p2 = b.goal;
    auto p = move(b.pos, b.goal, b.speed);
    b.pos = p;
    if (p.x == p2.x && p.y == p2.y) {
      // 删除ball
      return false;
    }
  }
  return true;
}

bool ball_logic_system::run() {
  auto now = window::dt_time;
  for (auto &bs : ball_game_instance::data) {
    // 使用迭代器遍历并安全删除
    for (auto it = bs.begin(); it != bs.end();) {
      if (it->delay >= now) {
        it++;
        continue;
      }

      run_animate(*it);
      auto r = run_move(*it);
      if (!r) {
        it = bs.erase(it); // erase 返回下一个迭代器
        continue;
      }
      it++;
    }
  }

  return true;
}