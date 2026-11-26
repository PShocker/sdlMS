#include "src/client/system/ui/pet_info_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/pet_game_instance.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <format>
#include <string>

namespace sdlms {

namespace {

// 布局全部取自 UI.wz/CharacterInfo.img/PetInfo 的 vector:*。
constexpr SDL_FPoint kTypeLt{208, 8};
constexpr SDL_FPoint kLevelLt{208, 27};
constexpr SDL_FPoint kClosenessLt{208, 46};
constexpr SDL_FPoint kFullnessLt{208, 65};
constexpr SDL_FPoint kPetCb{67, 90};
constexpr SDL_FPoint kNameLt{67, 100};
constexpr SDL_FPoint kEquipLt{129, 85};

}  // namespace

namespace {

// 宠物信息窗口显示的是"当前角色信息窗口里那个角色"的宠物（自己或别人）。
bool LoadViewedPet(GamePet& pet, const GameCharacter** owner) {
  const auto& viewed = CharacterInfoUiSystem::character_;
  if (viewed.name.empty()) {
    return false;
  }
  const auto& self = CharacterGameInstance::self_;
  if (viewed.name == self.name) {
    if (PetGameInstance::data_.empty()) {
      return false;
    }
    pet = PetGameInstance::data_.front();
    *owner = &self;
    return true;
  }
  for (auto&& [client_id, other] : CharacterGameInstance::others_) {
    if (other.g_character.name != viewed.name) {
      continue;
    }
    auto it = PetGameInstance::others_.find(client_id);
    if (it == PetGameInstance::others_.end()) {
      return false;
    }
    pet = it->second;
    *owner = &other.g_character;
    return true;
  }
  return false;
}

}  // namespace

SDL_FPoint PetInfoUiSystem::LoadWh() {
  static auto back = WzResource::LoadTexture(
      WzResource::ui_->find(u"CharacterInfo.img/PetInfo/backgrnd_detail"));
  return {static_cast<float>(back->w), static_cast<float>(back->h)};
}

bool PetInfoUiSystem::CursorIn() {
  auto wh = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, wh.x, wh.y};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void PetInfoUiSystem::RenderBackgrnd() {
  static auto back = WzResource::LoadTexture(
      WzResource::ui_->find(u"CharacterInfo.img/PetInfo/backgrnd_detail"));
  SDL_FRect pos_rect{
      static_cast<float>(static_cast<int>(pos_.x)),
      static_cast<float>(static_cast<int>(pos_.y)),
      static_cast<float>(back->w),
      static_cast<float>(back->h),
  };
  SDL_RenderTexture(Window::renderer_, back, nullptr, &pos_rect);
}

void PetInfoUiSystem::RenderPet() {
  GamePet pet;
  const GameCharacter* owner = nullptr;
  if (!LoadViewedPet(pet, &owner)) {
    return;
  }
  auto* node = PetGameInstance::LoadPetNode(pet.id);
  if (node == nullptr) {
    return;
  }
  // 095 CUI_PetInfo：宠物用它的动作帧画在 vector:pet (67,90) 的脚底位置。
  auto action = pet.action.empty() ? std::u16string(u"stand0") : pet.action;
  auto* action_node = node->get_child(action);
  auto* frame_node = action_node == nullptr
                         ? nullptr
                         : action_node->get_child(std::to_string(pet.ani_index));
  if (frame_node == nullptr && action_node != nullptr) {
    frame_node = action_node->get_child(u"0");
  }
  if (frame_node != nullptr && frame_node->type == wz::Type::UOL) {
    frame_node = static_cast<wz::Property<wz::WzUOL>*>(frame_node)->get_uol();
  }
  if (frame_node == nullptr || frame_node->get_child(u"origin") == nullptr) {
    return;
  }
  auto texture = WzResource::LoadTexture(frame_node);
  auto origin = WzResource::LoadFpoint(frame_node->get_child(u"origin"));
  SDL_FRect pos_rect{
      pos_.x + kPetCb.x - origin.x,
      pos_.y + kPetCb.y - origin.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect, 0,
                           nullptr,
                           pet.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void PetInfoUiSystem::RenderEquip() {
  GamePet pet;
  const GameCharacter* owner = nullptr;
  // 装备 id 随宠物消息同步（pet.equip_id），不再读主人角色的装备槽。
  (void)owner;
  if (!LoadViewedPet(pet, &owner) || pet.equip_id == 0) {
    return;
  }
  auto equip_text = std::format("{:08d}", pet.equip_id);
  auto* info = EquipGameInstance::LoadEquipInfo(
      std::u16string{equip_text.begin(), equip_text.end()});
  if (info == nullptr || info->get_child(u"icon") == nullptr) {
    return;
  }
  auto icon = WzResource::LoadTexture(info->get_child(u"icon"));
  SDL_FRect pos_rect{
      pos_.x + kEquipLt.x - icon->w / 2.0f,
      pos_.y + kEquipLt.y - icon->h / 2.0f,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
}

void PetInfoUiSystem::RenderText() {
  GamePet pet;
  const GameCharacter* owner = nullptr;
  if (!LoadViewedPet(pet, &owner)) {
    return;
  }
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);

  auto to_u16 = [](uint32_t v) {
    auto s = std::to_string(v);
    return std::u16string{s.begin(), s.end()};
  };

  auto draw_value = [](const std::u16string& text, SDL_FPoint at) {
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(text, static_cast<int>(at.x), static_cast<int>(at.y));
  };

  // 类型 = 宠物模板名（String.wz/Pet.img/<id>/name）
  draw_value(PetGameInstance::LoadPetName(pet.id),
             {pos_.x + kTypeLt.x, pos_.y + kTypeLt.y});
  draw_value(to_u16(pet.level), {pos_.x + kLevelLt.x, pos_.y + kLevelLt.y});
  auto closeness = static_cast<uint32_t>(pet.closeness);
  draw_value(to_u16(closeness),
             {pos_.x + kClosenessLt.x, pos_.y + kClosenessLt.y});
  draw_value(to_u16(pet.fullness),
             {pos_.x + kFullnessLt.x, pos_.y + kFullnessLt.y});

  // 名字画在宠物图标下面，居中对齐。
  auto w = Freetype::LoadW(pet.name);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(pet.name,
                     static_cast<int>(pos_.x + kNameLt.x - w / 2),
                     static_cast<int>(pos_.y + kNameLt.y));
}

UIComponents& PetInfoUiSystem::GetComponent() {
  components_.clear();
  auto wh = LoadWh();
  auto hide_node = WzResource::ui_->find(u"CharacterInfo.img/PetInfo/button:hide");
  SDL_FRect rect{
      static_cast<float>(static_cast<int>(pos_.x + wh.x - 20)),
      static_cast<float>(static_cast<int>(pos_.y + 7)),
      12,
      12,
  };
  Button button{.node = hide_node,
                .rect = rect,
                .enable = true,
                .focused = CursorGameInstance::cursor_ui_ == Render &&
                           !CursorGameInstance::modal_overlay_};
  components_.push_back(button);
  std::get<Button>(components_.back()).callback = EventClose;
  return components_;
}

bool PetInfoUiSystem::Render() {
  RenderBackgrnd();
  RenderPet();
  RenderEquip();
  RenderText();
  RenderUI();
  return true;
}

void PetInfoUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void PetInfoUiSystem::Open() {
  auto it = std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it == System::render_systems_.end()) {
    return;
  }
  auto wh = LoadWh();
  auto& camera = CameraGameInstance::camera_;
  pos_.x = (camera.w - wh.x) / 2;
  pos_.y = (camera.h - wh.y) / 2;
  System::render_systems_.insert(it, Render);
  System::event_systems_.insert(System::event_systems_.begin(), Event);
}

void PetInfoUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  drag_ = std::nullopt;
}

void PetInfoUiSystem::EventClose() {
  Close();
}

void PetInfoUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect bar{pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse{event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse, &bar)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
}

void PetInfoUiSystem::EventDragMove(SDL_Event* event) {
  if (!drag_.has_value()) {
    return;
  }
  pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
  auto& camera = CameraGameInstance::camera_;
  auto wh = LoadWh();
  pos_.x = std::clamp(pos_.x, 0.0f, camera.w - wh.x);
  pos_.y = std::clamp(pos_.y, 0.0f, camera.h - wh.y);
}

bool PetInfoUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
        EventClose();
        return false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT &&
          CursorGameInstance::cursor_ui_ == Render) {
        EventDragStart(event);
        r = false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT &&
          CursorGameInstance::cursor_ui_ == Render) {
        if (!drag_.has_value()) {
          r = !DispatchButtonClick(GetComponent(), true);
        }
        drag_ = std::nullopt;
        return false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventDragMove(event);
      break;
    }
    default: {
      break;
    }
  }
  return r;
}

}  // namespace sdlms
