#include "src/client/system/ui/party_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/party_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <wz/Property.h>
#include <algorithm>
namespace sdlms {


void PartyUiSystem::RenderBackgrnd(float x, float y, float w, float h) {
  // backgrnd
  static auto backgrnd_node =
      WzResource::ui_->find(u"UIToolTip.img/Skill/Frame");

  auto texture_c = WzResource::LoadTexture(backgrnd_node->get_child(u"c"));
  auto texture_e =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"e")));
  auto texture_ne =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"ne")));
  auto texture_n =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"n")));
  auto texture_nw =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"nw")));
  auto texture_w =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"w")));
  auto texture_sw =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"sw")));
  auto texture_s =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"s")));
  auto texture_se =
      WzResource::LoadTexture(static_cast<wz::Property<wz::WzCanvas> *>(
          backgrnd_node->get_child(u"se")));

  SDL_FRect rect;
  rect.x = x;
  rect.y = y;

  // 左上
  SDL_FPoint p_nw{x, y};
  rect.x = (p_nw.x);
  rect.y = (p_nw.y);
  rect.w = texture_nw->w;
  rect.h = texture_nw->h;
  SDL_RenderTexture(Window::renderer_, texture_nw, nullptr, &rect);

  // 左下
  SDL_FPoint p_sw{
      x,
      y + h - texture_sw->h,
  };
  rect.x = (p_sw.x);
  rect.y = (p_sw.y);
  rect.w = texture_sw->w;
  rect.h = texture_sw->h;
  SDL_RenderTexture(Window::renderer_, texture_sw, nullptr, &rect);

  // 右上
  SDL_FPoint p_ne{
      x + w - texture_ne->w,
      y,
  };
  rect.x = (p_ne.x);
  rect.y = (p_ne.y);
  rect.w = texture_ne->w;
  rect.h = texture_ne->h;
  SDL_RenderTexture(Window::renderer_, texture_ne, nullptr, &rect);

  // 右下
  SDL_FPoint p_se{
      x + w - texture_ne->w,
      y + h - texture_sw->h,
  };
  rect.x = (p_se.x);
  rect.y = (p_se.y);
  rect.w = texture_se->w;
  rect.h = texture_se->h;
  SDL_RenderTexture(Window::renderer_, texture_se, nullptr, &rect);

  rect.x = (x + texture_nw->w);
  rect.y = (y + texture_nw->h);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(Window::renderer_, texture_c, nullptr, 1, &rect);

  // 竖着的两边
  rect.x = (p_nw.x);
  rect.y = (p_nw.y + texture_nw->h);
  rect.w = texture_w->w;
  rect.h = p_sw.y - (p_nw.y + texture_nw->h);
  SDL_RenderTextureTiled(Window::renderer_, texture_w, nullptr, 1, &rect);

  rect.x = (p_se.x);
  rect.y = (p_ne.y + texture_ne->h);
  rect.w = texture_e->w;
  rect.h = p_se.y - (p_ne.y + texture_ne->h);
  SDL_RenderTextureTiled(Window::renderer_, texture_e, nullptr, 1, &rect);

  // 横着的两边
  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_ne.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_n->h;
  SDL_RenderTextureTiled(Window::renderer_, texture_n, nullptr, 1, &rect);

  rect.x = (p_nw.x + texture_nw->w);
  rect.y = (p_sw.y);
  rect.w = p_ne.x - (p_nw.x + texture_nw->w);
  rect.h = texture_s->h;
  SDL_RenderTextureTiled(Window::renderer_, texture_s, nullptr, 1, &rect);
}

void PartyUiSystem::RenderPartyHp() {
  for (int i = 0; i < PartyGameInstance::data_->clients.size(); i++) {
    const auto &player = PartyGameInstance::data_->clients[i];
    static auto texture = WzResource::LoadTexture(WzResource::ui_->find(u""));
  }
}

bool PartyUiSystem::Render() {
  auto [w, h] = LoadWh();
  RenderBackgrnd(0, 0, w, h);
  return true;
}

SDL_FPoint PartyUiSystem::LoadWh() { return {209, 289}; }

void PartyUiSystem::Open() {
  auto it = std::ranges::find(System::render_systems_, &Render);
  if (it != System::render_systems_.end()) {
    return;
  }
  it = std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void PartyUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void PartyUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void PartyUiSystem::EventDragStart(SDL_Event *Event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {Event->button.x, Event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - Event->button.x, pos_.y - Event->button.y};
  }
  return;
}

void PartyUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void PartyUiSystem::EventDragMove(SDL_Event *Event) {
  if (drag_.has_value()) {
    pos_ = {Event->motion.x + drag_->x, Event->motion.y + drag_->y};
    auto &camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void PartyUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool PartyUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool PartyUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventTop();
        EventDragStart(Event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
      }
      EventDragEnd();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    EventDragMove(Event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

}  // namespace sdlms
