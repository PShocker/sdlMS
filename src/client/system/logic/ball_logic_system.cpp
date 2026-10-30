#include "src/client/system/logic/ball_logic_system.h"

#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace sdlms {

void BallLogicSystem::RunAnimate(GameBall& b) {
  wz::Node* n = WzResource::skill_->find(b.path);
  if (!n) {
    n = WzResource::item_->find(b.path);
  }
  b.ani_time += Window::delta_time_;
  auto index = std::to_string(b.ani_index);
  auto texture_node = n->get_child(index);
  int delay = 100;
  if (texture_node->get_child(u"delay")) {
    delay = static_cast<wz::Property<int>*>(texture_node->get_child(u"delay"))
                ->get();
  }
  if (b.ani_time >= delay) {
    b.ani_time = 0;
    b.ani_index += 1;
    auto m = n->children;
    if (m.contains(u"rotatePeriod")) {
      auto r = static_cast<wz::Property<int>*>(m.at(u"rotatePeriod")[0])->get();
      b.rotate += r;
      m.erase(u"rotatePeriod");
    }
    b.ani_index = b.ani_index % m.size();
  }
  // 095 的子弹 origin 挂在目标 Vector2D 上，但发射时的始/终点偏移固定。
  // 因此这里用固定的相对弹道，不能在每帧朝怪物当前位置重新瞄准。
  SDL_FPoint from = b.start_offset;
  SDL_FPoint to = b.goal;
  if (!MobGameInstance::data_.contains(b.mob_index)) {
    from = b.start;
    to = b.spawn_goal;
  }

  float dx = to.x - from.x;
  float dy = to.y - from.y;
  float angle = SDL_atan2f(dy, dx) * (180.0f / std::numbers::pi);
  if (dx < 0) {
    angle += 180.0f;
    angle -= (angle > 180.0f) ? 360.0f : 0.0f;
  }
  b.rotate = angle;
  return;
}

SDL_FPoint BallLogicSystem::Move(SDL_FPoint p1, SDL_FPoint p2, int speed) {
  auto step = speed * Window::delta_time_ / 1000.0f;

  float dx = p2.x - p1.x;
  float dy = p2.y - p1.y;
  float length = sqrtf(dx * dx + dy * dy);

  // 如果剩余距离 <= 步长，直接到达终点并停止
  if (length <= step) {
    return p2;
  }

  // 否则正常移动 step 距离
  float norm_x = dx / length;
  float norm_y = dy / length;

  SDL_FPoint result;
  result.x = p1.x + norm_x * step;
  result.y = p1.y + norm_y * step;

  return result;
}

bool BallLogicSystem::RunMove(GameBall& b) {
  auto now = Window::dt_time_;
  if (b.move_start == 0) {
    b.move_start = now;
  }
  if (b.duration == 0) {
    b.duration = 1;
  }

  // 目标存在时整条弹道跟随目标平移；目标消失后保留发射时的绝对弹道。
  SDL_FPoint from = b.start;
  SDL_FPoint to = b.spawn_goal;
  if (MobGameInstance::data_.contains(b.mob_index)) {
    const auto mob_pos = MobGameInstance::data_.at(b.mob_index).mob.pos;
    from = {
        b.start_offset.x + mob_pos.x,
        b.start_offset.y + mob_pos.y,
    };
    to = {
        b.goal.x + mob_pos.x,
        b.goal.y + mob_pos.y,
    };
  }

  const auto elapsed = now - b.move_start;
  const float t =
      std::min(1.0f, static_cast<float>(elapsed) / static_cast<float>(b.duration));
  b.pos = {
      from.x + (to.x - from.x) * t,
      from.y + (to.y - from.y) * t,
  };

  // 到达终点后删除 ball
  return t < 1.0f;
}

bool BallLogicSystem::Run() {
  auto now = Window::dt_time_;
  for (auto& bs : BallGameInstance::data_) {
    // 使用迭代器遍历并安全删除
    for (auto it = bs.begin(); it != bs.end();) {
      if (it->delay >= now) {
        it++;
        continue;
      }

      RunAnimate(*it);
      auto r = RunMove(*it);
      if (!r) {
        it = bs.erase(it);  // erase 返回下一个迭代器
        continue;
      }
      it++;
    }
  }

  return true;
}

}  // namespace sdlms
