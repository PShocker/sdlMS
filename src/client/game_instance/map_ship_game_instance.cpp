#include "src/client/game_instance/map_ship_game_instance.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_render.h>
#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace sdlms {

namespace {

int LoadInt(wz::Node* node, const char16_t* name, int fallback) {
  auto* child = node->get_child(name);
  if (child == nullptr || child->type != wz::Type::Int) {
    return fallback;
  }
  return static_cast<wz::Property<int32_t>*>(child)->get();
}

// shipObj 路径形如 Map/Obj/obj_vehicle.img/ship/ossyria/99；
// 部分地图省略 .img（Map/Obj/obj_vehicle/ship/...），补回后再查。
wz::Node* FindShipNode(std::u16string path) {
  if (path.starts_with(u"Map/")) {
    path = path.substr(4);
  }
  if (auto* node = WzResource::map_->find(path)) {
    return node;
  }
  auto slash = path.find(u'/');
  if (slash == std::u16string::npos) {
    return nullptr;
  }
  auto first = path.substr(0, slash);
  if (first.ends_with(u".img")) {
    return nullptr;
  }
  return WzResource::map_->find(first + u".img/" + path.substr(slash + 1));
}

}  // namespace

void MapShipGameInstance::Clear() { ship_ = GameShip{}; }

void MapShipGameInstance::Load(uint32_t map_id) {
  ship_ = GameShip{};

  auto* map_node = WzResource::LoadMapNode(map_id);
  if (map_node == nullptr) {
    return;
  }
  auto* node = map_node->get_child(u"shipObj");
  if (node == nullptr) {
    return;
  }

  GameShip ship;
  ship.kind = LoadInt(node, u"shipKind", 0);
  ship.t_move = LoadInt(node, u"tMove", 0);
  ship.x = LoadInt(node, u"x", 0);
  ship.y = LoadInt(node, u"y", 0);
  ship.x0 = LoadInt(node, u"x0", ship.x);
  ship.pos = SDL_FPoint{static_cast<float>(ship.x),
                        static_cast<float>(ship.y)};

  auto* path_node = node->get_child(u"shipObj");
  if (path_node == nullptr || path_node->type != wz::Type::String) {
    return;
  }
  auto path =
      static_cast<wz::Property<std::u16string>*>(path_node)->get();
  auto* ship_node = FindShipNode(path);
  if (ship_node == nullptr) {
    return;
  }
  auto* sprite = ship_node->get_child(u"0");
  if (sprite == nullptr) {
    sprite = ship_node;
  }
  ship.sprite = sprite;
  if (auto* origin = sprite->get_child(u"origin"); origin != nullptr) {
    ship.origin = WzResource::LoadFpoint(origin);
  }

  auto now = Window::dt_now_;
  if (ship.kind == 0) {
    // 095 CShip::EnterShipMove：从 x0 用 tMove 秒移动到 x。
    ship.from = SDL_FPoint{static_cast<float>(ship.x0),
                           static_cast<float>(ship.y)};
    ship.to = SDL_FPoint{static_cast<float>(ship.x),
                         static_cast<float>(ship.y)};
    ship.pos = ship.from;
    ship.move_start = now;
    ship.move_duration =
        static_cast<uint64_t>(std::max(0, ship.t_move)) * 1000;
  } else {
    // 095 CShip::MoveShip：停泊船在 ±50/±100 范围内随机漂移。
    ship.from = ship.pos;
    ship.to = ship.pos;
    ship.move_start = now;
    ship.move_duration = 0;
    ship.next_move = now + std::rand() % 1000 + 1000;
  }

  ship.exists = true;
  ship_ = ship;
}

bool MapShipGameInstance::Run() {
  if (!ship_.exists) {
    return true;
  }
  auto now = Window::dt_now_;

  if (ship_.kind == 0) {
    // 线性行驶；走完后停在终点。
    if (ship_.move_duration == 0) {
      ship_.pos = ship_.to;
      return true;
    }
    auto t = static_cast<float>(now - ship_.move_start) /
             static_cast<float>(ship_.move_duration);
    t = std::clamp(t, 0.0f, 1.0f);
    ship_.pos.x = ship_.from.x + (ship_.to.x - ship_.from.x) * t;
    ship_.pos.y = ship_.from.y + (ship_.to.y - ship_.from.y) * t;
    return true;
  }

  // shipKind 1：每 1~2 秒随机移动 20~30px，限制在 ±50/±100 内。
  if (now >= ship_.next_move) {
    auto dx = std::rand() % 11 + 20;
    auto dy = std::rand() % 11 + 20;
    auto tx = ship_.to.x + (std::rand() % 2 == 0 ? -dx : dx);
    auto ty = ship_.to.y + (std::rand() % 2 == 0 ? -dy : dy);
    tx = std::clamp(tx, static_cast<float>(ship_.x - 50),
                    static_cast<float>(ship_.x + 50));
    ty = std::clamp(ty, static_cast<float>(ship_.y - 100),
                    static_cast<float>(ship_.y + 100));
    ship_.from = ship_.pos;
    ship_.to = SDL_FPoint{tx, ty};
    ship_.move_start = now;
    ship_.move_duration = static_cast<uint64_t>(std::rand() % 1000 + 1000);
    ship_.next_move = now + ship_.move_duration;
  }
  if (ship_.move_duration != 0) {
    auto t = static_cast<float>(now - ship_.move_start) /
             static_cast<float>(ship_.move_duration);
    t = std::clamp(t, 0.0f, 1.0f);
    ship_.pos.x = ship_.from.x + (ship_.to.x - ship_.from.x) * t;
    ship_.pos.y = ship_.from.y + (ship_.to.y - ship_.from.y) * t;
  }
  return true;
}

bool MapShipGameInstance::Render() {
  if (!ship_.exists || ship_.sprite == nullptr) {
    return true;
  }
  auto* texture = WzResource::LoadTexture(ship_.sprite);
  if (texture == nullptr) {
    return true;
  }
  auto& camera = CameraGameInstance::camera_;
  SDL_FRect dst{
      ship_.pos.x - ship_.origin.x - camera.x,
      ship_.pos.y - ship_.origin.y - camera.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &dst);
  return true;
}

}  // namespace sdlms
