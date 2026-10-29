#include "src/client/system/ui/worldmap_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

void WorldmapUiSystem::RenderBackgrnd() {
  auto wh = LoadWh();
  auto w = wh.x;
  auto h = wh.y;
  static auto bord_node = WzResource::ui_->find(u"WorldMap.img/Border");
  static auto lt = WzResource::LoadTexture(bord_node->get_child(u"0"));
  static auto t = WzResource::LoadTexture(bord_node->get_child(u"1"));
  static auto rt = WzResource::LoadTexture(bord_node->get_child(u"2"));
  static auto lm = WzResource::LoadTexture(bord_node->get_child(u"3"));
  static auto rm = WzResource::LoadTexture(bord_node->get_child(u"4"));
  static auto lb = WzResource::LoadTexture(bord_node->get_child(u"5"));
  static auto b = WzResource::LoadTexture(bord_node->get_child(u"6"));
  static auto rb = WzResource::LoadTexture(bord_node->get_child(u"7"));
  static auto title =
      WzResource::LoadTexture(WzResource::ui_->find(u"WorldMap.img/title"));

  SDL_FRect pos_rect = {pos_.x, pos_.y, (float)lt->w, (float)lt->h};
  SDL_RenderTexture(Window::renderer_, lt, nullptr, &pos_rect);
  pos_rect = {pos_.x + 7, pos_.y, static_cast<float>(w - 14), (float)t->h};
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  pos_rect = {pos_.x + w - 7, pos_.y, static_cast<float>(rt->w), (float)t->h};
  SDL_RenderTexture(Window::renderer_, rt, nullptr, &pos_rect);
  pos_rect = {pos_.x, pos_.y + 32, static_cast<float>(lm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(Window::renderer_, lm, nullptr, &pos_rect);
  pos_rect = {pos_.x + w - 7, pos_.y + 32, static_cast<float>(rm->w),
              (float)h - b->h - 32};
  SDL_RenderTexture(Window::renderer_, rm, nullptr, &pos_rect);
  pos_rect = {pos_.x, pos_.y + h - b->h, static_cast<float>(lb->w),
              (float)lb->h};
  SDL_RenderTexture(Window::renderer_, lb, nullptr, &pos_rect);
  pos_rect = {pos_.x + 7, pos_.y + h - b->h, static_cast<float>(w - 14),
              (float)b->h};
  SDL_RenderTexture(Window::renderer_, b, nullptr, &pos_rect);
  pos_rect = {pos_.x + w - 7, pos_.y + h - b->h, static_cast<float>(rb->w),
              (float)rb->h};
  SDL_RenderTexture(Window::renderer_, rb, nullptr, &pos_rect);
  pos_rect = {static_cast<float>((int32_t)pos_.x + 10),
              static_cast<float>((int32_t)pos_.y + 10),
              static_cast<float>(title->w), (float)title->h};
  SDL_RenderTexture(Window::renderer_, title, nullptr, &pos_rect);
}

static const SDL_FPoint map_offset = {6, 30};

static uint32_t spot_info_id;

void WorldmapUiSystem::RenderSpot() {
  struct WorldmapSpot {
    SDL_FPoint pos;
    uint8_t type;
    std::flat_set<uint32_t> map_id;
  };
  spot_info_id = 0;
  static std::flat_map<std::u16string, std::vector<WorldmapSpot>> spot_cache;
  if (!spot_cache.contains(path_)) {
    auto spot_node = WzResource::map_->find(u"WorldMap/" + path_ + u"/MapList");
    for (auto [k, v] : *spot_node->get_children()) {
      WorldmapSpot wsp;
      auto vec =
          static_cast<wz::Property<wz::WzVec2D>*>(v[0]->get_child(u"spot"))
              ->get();
      wsp.pos = {static_cast<float>(vec.x), static_cast<float>(vec.y)};
      wsp.type =
          static_cast<wz::Property<int>*>(v[0]->get_child("type"))->get();
      for (auto [m, mid] : *v[0]->get_child(u"mapNo")) {
        auto map_id = static_cast<wz::Property<int>*>(mid[0])->get();
        wsp.map_id.insert(map_id);
      }
      spot_cache[path_].push_back(wsp);
    }
  }
  auto spots = spot_cache.at(path_);
  static auto texture_node = WzResource::map_->find("MapHelper.img/worldMap");
  static std::array map_img_array = {
      WzResource::LoadTexture(texture_node->find(u"mapImage/0")),
      WzResource::LoadTexture(texture_node->find(u"mapImage/1")),
      WzResource::LoadTexture(texture_node->find(u"mapImage/2")),
  };

  auto wh = LoadWh();
  auto map_ox = (wh.x - 12) / 2;
  auto map_oy = (wh.y - 44) / 2;

  auto& mouse_pos = Window::mouse_pos_;
  std::optional<SDL_FPoint> spot_point;
  for (auto& spot : spots) {
    auto texture = map_img_array[spot.type];
    SDL_FRect pos_rect = {
        static_cast<float>(int(pos_.x + map_ox + spot.pos.x + map_offset.x -
                               (float)texture->w / 2)),
        static_cast<float>(int(pos_.y + map_oy + spot.pos.y + map_offset.y -
                               (float)texture->h / 2)),
        static_cast<float>(texture->w), (float)texture->h};
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
    pos_rect.x -= 5;
    pos_rect.y -= 5;
    pos_rect.w += 5;
    pos_rect.h += 5;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      spot_info_id = *spot.map_id.begin();
    }
    if (spot.map_id.contains(SceneSystemInstance::map_id_)) {
      spot_point = {
          pos_rect.x + 5 + (float)texture->w / 2,
          pos_rect.y + 5,
      };
    }
  }
  if (spot_point.has_value()) {
    RenderCurPos(spot_point.value());
  }
}

void WorldmapUiSystem::RenderMap() {
  auto back_node = WzResource::map_->find(u"WorldMap/" + path_ + u"/BaseImg/0");
  auto texture = WzResource::LoadTexture(back_node);
  SDL_FRect pos_rect = {pos_.x + map_offset.x, pos_.y + map_offset.y,
                        static_cast<float>(texture->w), (float)texture->h};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

void WorldmapUiSystem::RenderCurPos(SDL_FPoint point) {
  auto now = Window::dt_now_;
  static auto point_node =
      WzResource::map_->find(u"MapHelper.img/worldMap/curPos");
  auto sum = point_node->children_count() * 200;
  auto offset = now % sum;  // 取余，得到周期内偏移
  auto i = offset / 200;
  auto index = std::to_string(i);
  auto node = point_node->get_child(index);
  auto origin = WzResource::LoadFpoint(node->get_child(u"origin"));
  auto texture = WzResource::LoadTexture(node);
  SDL_FRect pos_rect{
      point.x - origin.x,
      point.y - origin.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

bool WorldmapUiSystem::RenderSpotInfo() {
  if (CursorGameInstance::modal_overlay_ == nullptr &&
      CursorGameInstance::cursor_ui_ == Render && spot_info_id != 0) {
    const auto& mouse_pos = Window::mouse_pos_;
    TooltipUiSystem::RenderWorldMapInfo(spot_info_id, mouse_pos.x, mouse_pos.y);
  }
  return true;
}

UIComponents& WorldmapUiSystem::GetComponent() {
  const static std::array<Button::Callback, 1> kCallbacks = {
      EventClose,
  };

  components_.clear();
  // 构建按钮列表
  std::vector<wz::Node*> nodes = {
      WzResource::ui_->find(u"Basic.img/BtClose"),
  };
  auto [w, h] = LoadWh();
  std::vector<SDL_FRect> rects = {
      {w - 18, 6, 12, 12},  //
  };
  // 渲染所有按钮

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = rects[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    Button button{.node = nodes[i],
                  .rect = rect,
                  .enable = true,
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             !CursorGameInstance::modal_overlay_};
    components_.push_back(button);
    if (i < kCallbacks.size()) {
      std::get<Button>(components_.back()).callback = kCallbacks[i];
    }
  }
  return components_;
}


void WorldmapUiSystem::RenderUI() {
  RenderComponents(GetComponent());
}


bool WorldmapUiSystem::Render() {
  RenderBackgrnd();
  RenderMap();
  RenderSpot();
  RenderUI();
  return true;
}

SDL_FPoint WorldmapUiSystem::LoadWh() {
  static std::flat_map<std::u16string, SDL_FPoint> whs;
  if (!whs.contains(path_)) {
    auto back_node =
        WzResource::map_->find(u"WorldMap/" + path_ + u"/BaseImg/0");
    auto texture = WzResource::LoadTexture(back_node);
    whs[path_] = SDL_FPoint{static_cast<float>(texture->w + 12),
                            static_cast<float>(texture->h + 44)};
  }
  return whs.at(path_);
}

void WorldmapUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    if (MinimapUiSystem::disable_) {
      NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kWorldmapDisable;
      NoticeUiSystem::Close();
      NoticeUiSystem::Open();
      return;
    }
    WorldmapUiSystem::path_ = u"WorldMap000.img";

    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void WorldmapUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::render_systems_, RenderSpotInfo);
  std::erase(System::event_systems_, Event);

  EventDragEnd();
}

void WorldmapUiSystem::EventClose() {
  Close();
}

void WorldmapUiSystem::EventClickSpot() {
  if (spot_info_id != 0) {
    NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kWorldmapTeleport;
    NoticeUiSystem::data_ = spot_info_id;
    NoticeUiSystem::Close();
    NoticeUiSystem::Open();
  }
}

bool WorldmapUiSystem::EventUI(SDL_Event* event) {
  if (DispatchButtonClick(GetComponent(), false)) {
    return true;
  }
  EventClickSpot();
  return false;
}


void WorldmapUiSystem::EventMotion(SDL_Event* event) {
  auto& sys = System::render_systems_;
  std::erase(sys, RenderSpotInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderSpotInfo);
  }
}

bool WorldmapUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      switch (scan_code) {
        case SDL_SCANCODE_ESCAPE: {
          EventClose();
          return false;
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventTop();
          EventDragStart(event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        r = !EventUI(event);
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotion(event);
      EventDragMove(event);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

bool WorldmapUiSystem::CursorIn() {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, wh.y};
  return SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect);
}

void WorldmapUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void WorldmapUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void WorldmapUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
  return;
}

void WorldmapUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void WorldmapUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

}  // namespace sdlms
