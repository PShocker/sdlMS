#include "quick_slot_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_input.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <flat_map>
#include <format>
#include <optional>
#include <string>

void QuickSlotUiSystem::render_input_icon(const GameInput &i, int x, int y,
                                             uint8_t alpha) {
  SDL_Texture *t;
  std::u16string id;
  if (i.type == "skill") {
    id = {i.val.begin(), i.val.end()};
    auto ski_info = SkillGameInstance::load_ski_node(id);
    t = WzResource::load_texture(ski_info->get_child(u"icon"));
  } else if (i.type == "item") {
    id = {i.val.begin(), i.val.end()};
    auto item_info = ItemGameInstance::load_item_info(id, 1);
    t = WzResource::load_texture(item_info->get_child(u"icon"));
  } else if (i.type == "action") {
    const static std::flat_map<std::string, SDL_Texture *> actions = {
        {"pick", WzResource::load_texture(
                     WzResource::ui->find(u"KeyBindings.img/icon/50"))},
        {"sit", WzResource::load_texture(
                    WzResource::ui->find(u"KeyBindings.img/icon/51"))},
        {"attack", WzResource::load_texture(
                       WzResource::ui->find(u"KeyBindings.img/icon/52"))},
        {"jump", WzResource::load_texture(
                     WzResource::ui->find(u"KeyBindings.img/icon/53"))},
        {"up", WzResource::load_texture(
                   WzResource::ui->find(u"KeyBindings.img/icon/302"))},
        {"down", WzResource::load_texture(
                     WzResource::ui->find(u"KeyBindings.img/icon/303"))},
        {"left", WzResource::load_texture(
                     WzResource::ui->find(u"KeyBindings.img/icon/300"))},
        {"right", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/301"))},
        {"harvest", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/54"))},
    };
    if (actions.contains(i.val)) {
      t = actions.at(i.val);
    }
  } else if (i.type == "ui") {
    const static std::flat_map<std::string, SDL_Texture *> uis = {
        {"equip", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/0"))},
        {"package", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/1"))},
        {"characterStat", WzResource::load_texture(WzResource::ui->find(
                              u"KeyBindings.img/icon/2"))},
        {"skill", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/3"))},
        {"buddy", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/4"))},
        {"worldMap", WzResource::load_texture(
                         WzResource::ui->find(u"KeyBindings.img/icon/5"))},
        {"mapleChat", WzResource::load_texture(
                          WzResource::ui->find(u"KeyBindings.img/icon/6"))},
        {"miniMap", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/7"))},
        {"quest", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/8"))},
        {"keyBind", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/9"))},
        {"toAll", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/10"))},
        {"whisper", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/11"))},
        {"toParty", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/12"))},
        {"toBuddy", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/13"))},
        {"shortCut", WzResource::load_texture(
                         WzResource::ui->find(u"KeyBindings.img/icon/14"))},
        {"quickSlot", WzResource::load_texture(
                          WzResource::ui->find(u"KeyBindings.img/icon/15"))},
        {"chat+", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/16"))},
        {"party", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/19"))},
        {"cashShop", WzResource::load_texture(
                         WzResource::ui->find(u"KeyBindings.img/icon/22"))},
        {"changeChannel", WzResource::load_texture(WzResource::ui->find(
                              u"KeyBindings.img/icon/45"))},
        {"menu", WzResource::load_texture(
                     WzResource::ui->find(u"KeyBindings.img/icon/46"))},
        {"screenShot", WzResource::load_texture(
                           WzResource::ui->find(u"KeyBindings.img/icon/47"))},
        {"picMode", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/48"))},
        {"craft", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/55"))},
        {"mailBox", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/58"))},
        {"ok", WzResource::load_texture(
                   WzResource::ui->find(u"KeyBindings.img/icon/400"))},
    };
    if (uis.contains(i.val)) {
      t = uis.at(i.val);
    }
  } else if (i.type == "face") {
    const static std::flat_map<std::string, SDL_Texture *> faces = {
        {"hit", WzResource::load_texture(
                    WzResource::ui->find(u"KeyBindings.img/icon/100"))},
        {"smile", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/101"))},
        {"troubled", WzResource::load_texture(
                         WzResource::ui->find(u"KeyBindings.img/icon/102"))},
        {"cry", WzResource::load_texture(
                    WzResource::ui->find(u"KeyBindings.img/icon/103"))},
        {"angry", WzResource::load_texture(
                      WzResource::ui->find(u"KeyBindings.img/icon/104"))},
        {"bewildered", WzResource::load_texture(
                           WzResource::ui->find(u"KeyBindings.img/icon/105"))},
        {"stunned", WzResource::load_texture(
                        WzResource::ui->find(u"KeyBindings.img/icon/106"))},
    };
    if (faces.contains(i.val)) {
      t = faces.at(i.val);
    }
  }
  if (t == nullptr) {
    return;
  }
  SDL_FRect pos_rect{
      static_cast<float>(x),
      static_cast<float>(y),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  if (i.type == "item") {
    pos_rect.x += (32 - pos_rect.w) / 2;
    pos_rect.y += (32 - pos_rect.h) / 2;
  }
  SDL_SetTextureAlphaMod(t, alpha);
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
  SDL_SetTextureAlphaMod(t, 255);
  if (i.type == "item") {
    auto num = PackageGameInstance::load_item_num(id);
    PackageUiSystem::render_number(num, x, y + 16);
  }
}

void QuickSlotUiSystem::render_quickSlot() {
  const static std::flat_map<SDL_Scancode, SDL_Texture *> textures = {
      {SDL_SCANCODE_PAGEDOWN, WzResource::load_texture(WzResource::ui->find(
                                  u"QuickSlot.img/key/81"))},
      {SDL_SCANCODE_END, WzResource::load_texture(
                             WzResource::ui->find(u"QuickSlot.img/key/79"))},
      {SDL_SCANCODE_PAGEUP, WzResource::load_texture(WzResource::ui->find(
                                u"QuickSlot.img/key/73"))},
      {SDL_SCANCODE_HOME, WzResource::load_texture(
                              WzResource::ui->find(u"QuickSlot.img/key/71"))},
      {SDL_SCANCODE_INSERT, WzResource::load_texture(WzResource::ui->find(
                                u"QuickSlot.img/key/82"))},
      {SDL_SCANCODE_DELETE, WzResource::load_texture(WzResource::ui->find(
                                u"QuickSlot.img/key/83"))},
      {SDL_SCANCODE_LCTRL, WzResource::load_texture(
                               WzResource::ui->find(u"QuickSlot.img/key/29"))},
      {SDL_SCANCODE_LSHIFT, WzResource::load_texture(WzResource::ui->find(
                                u"QuickSlot.img/key/42"))},
  };
  switch (quickSlot) {
  case QuickSlot::hide: {
    break;
  }
  case QuickSlot::two: {
    static auto q = WzResource::load_texture(
        WzResource::ui->find(u"QuickSlot.img/layer:backgrnd"));
    auto screen_w = CameraGameInstance::camera.w;
    auto screen_h = CameraGameInstance::camera.h;
    auto base_x = (screen_w - 808) / 2;
    auto base_y = (screen_h - 73);
    SDL_FRect p{
        base_x + 654,
        base_y - 107,
        static_cast<float>(q->w),
        static_cast<float>(q->h),
    };
    SDL_RenderTexture(Window::renderer, q, nullptr, &p);
    std::vector<SDL_Scancode> scode = {
        (SDL_SCANCODE_LSHIFT), (SDL_SCANCODE_INSERT),   (SDL_SCANCODE_HOME),
        (SDL_SCANCODE_PAGEUP), (SDL_SCANCODE_LCTRL),    (SDL_SCANCODE_DELETE),
        (SDL_SCANCODE_END),    (SDL_SCANCODE_PAGEDOWN),
    };
    std::vector<SDL_FPoint> r = {
        SDL_FPoint{p.x + 9, p.y + 10},
        SDL_FPoint{p.x + 44, p.y + 10},
        SDL_FPoint{p.x + 79, p.y + 10},
        SDL_FPoint{p.x + 114, p.y + 10},
        // 2row
        SDL_FPoint{p.x + 9, p.y + 44},
        SDL_FPoint{p.x + 44, p.y + 44},
        SDL_FPoint{p.x + 79, p.y + 44},
        SDL_FPoint{p.x + 114, p.y + 44},
    };
    for (int i = 0; i < r.size(); i++) {
      SDL_FRect pos_rect;
      if (KeyboardGameInstance::data.contains(scode[i])) {
        const auto &input = KeyboardGameInstance::data.at(scode[i]);
        render_input_icon(input, r[i].x - 2, r[i].y + 32);
      }
      auto texture = textures.at(scode[i]);
      pos_rect = {
          r[i].x,
          r[i].y + 34,
          static_cast<float>(texture->w),
          static_cast<float>(texture->h),
      };
      SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
    }

    break;
  }
  case QuickSlot::three: {
    break;
  }
  }
}

bool QuickSlotUiSystem::render() {
  render_quickSlot();
  return true;
}

SDL_FPoint QuickSlotUiSystem::load_wh() {
  switch (quickSlot) {
  case QuickSlot::hide:
    return {0, 0};
  case QuickSlot::two:
    return {151, 78};
  case QuickSlot::three:
    return {151, 114};
  }
  return {0, 0};
}

void QuickSlotUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

void QuickSlotUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}

void QuickSlotUiSystem::toggle() {
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool QuickSlotUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  auto screen_w = CameraGameInstance::camera.w;
  auto screen_h = CameraGameInstance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  base_x = base_x + 654;
  base_y = base_y - 107;
  SDL_FRect pos_rect{
      base_x,
      base_y,
      w,
      h,
  };
  if (quickSlot == QuickSlot::two) {
    pos_rect.y += 36;
  }
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool QuickSlotUiSystem::event_click_quickslot(SDL_Event *event) {
  if (CursorGameInstance::cursor_hand_net.has_value()) {
    return false;
  }
  auto screen_w = CameraGameInstance::camera.w;
  auto screen_h = CameraGameInstance::camera.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  switch (quickSlot) {
  case QuickSlot::hide: {
    break;
  }
  case QuickSlot::two: {
    SDL_FPoint p{
        base_x + 654,
        base_y - 107,
    };
    std::vector<SDL_Scancode> t = {
        SDL_SCANCODE_LSHIFT, SDL_SCANCODE_INSERT,   SDL_SCANCODE_HOME,
        SDL_SCANCODE_PAGEUP, SDL_SCANCODE_LCTRL,    SDL_SCANCODE_DELETE,
        SDL_SCANCODE_END,    SDL_SCANCODE_PAGEDOWN,
    };
    std::vector<SDL_FPoint> r = {
        SDL_FPoint{p.x + 9, p.y + 10},
        SDL_FPoint{p.x + 44, p.y + 10},
        SDL_FPoint{p.x + 79, p.y + 10},
        SDL_FPoint{p.x + 114, p.y + 10},
        // 2row
        SDL_FPoint{p.x + 9, p.y + 44},
        SDL_FPoint{p.x + 44, p.y + 44},
        SDL_FPoint{p.x + 79, p.y + 44},
        SDL_FPoint{p.x + 114, p.y + 44},
    };
    for (int i = 0; i < r.size(); i++) {
      SDL_FRect pos_rect{
          r[i].x,
          r[i].y + 32,
          static_cast<float>(32),
          static_cast<float>(32),
      };
      if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
        if (CursorGameInstance::cursor_hand.has_value()) {
          auto input = KeyboardGameInstance::load_cursor_input();
          KeyboardGameInstance::insert_input(t[i], input.value());
          CursorGameInstance::cursor_hand = std::nullopt;
          AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
        } else {
          if (KeyboardGameInstance::data.contains(t[i])) {
            CursorGameInstance::cursor_hand = {
                .type = CursorGameInstance::keybind,
                .val = t[i],
            };
            AudioGameInstance::load_audio(u"UI.img/DragStart", 0);
          }
        }
        return true;
      }
    }

    break;
  }
  case QuickSlot::three: {
    break;
  }
  }
  return false;
}

bool QuickSlotUiSystem::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        r = !event_click_quickslot(event);
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}