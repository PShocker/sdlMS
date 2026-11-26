#include "src/client/system/render/chatballoon_render_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/system/render/pet_render_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/pet_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Property.h>
#include <wz/Wz.h>

#include <cmath>
#include <cstdint>
#include <flat_map>
#include <format>
#include <string>

namespace sdlms {

namespace {

// 宠物装备：Character.wz/PetEquip/<装备 id>.img/<宠物 id>/<动作>/<帧>。
// 装备图里每个动作/帧和宠物本体一一对应，用同一个 origin 叠在宠物身上画
// （095 CPet::GetItemSlot / LoadLayer 的宠物装备层）。装备 id 随宠物
// 消息同步（pet.equip_id）：自己的来自穿戴上报，其他玩家的来自
// ServerCharacterPet，不再读主人角色的装备槽。
void DrawPetEquip(const GamePet& pet, const GameCharacter& owner) {
  if (pet.equip_id == 0) {
    return;
  }
  auto equip_text = std::format("{:08d}", pet.equip_id);
  auto equip_id = std::u16string{equip_text.begin(), equip_text.end()};
  static std::flat_map<std::u16string, wz::Node*> cache;
  wz::Node* image = nullptr;
  if (auto it = cache.find(equip_id); it != cache.end()) {
    image = it->second;
  } else {
    image = WzResource::character_->find(u"PetEquip/" + equip_id + u".img");
    cache.emplace(equip_id, image);
  }
  if (image == nullptr) {
    return;
  }
  auto pet_key = PetGameInstance::LoadPetImageKey(pet.id);
  if (pet_key.empty()) {
    return;
  }
  auto* pet_node = image->get_child(pet_key);
  if (pet_node == nullptr) {
    // 装备图里没有这只宠物的分支：这只宠物穿不了这件装备。
    return;
  }
  auto* action_node = pet_node->get_child(pet.action);
  if (action_node == nullptr) {
    return;
  }
  auto* frame_node = action_node->get_child(std::to_string(pet.ani_index));
  if (frame_node == nullptr) {
    return;
  }
  if (frame_node->type == wz::Type::UOL) {
    frame_node = static_cast<wz::Property<wz::WzUOL>*>(frame_node)->get_uol();
  }
  if (frame_node == nullptr || frame_node->get_child(u"origin") == nullptr) {
    return;
  }
  auto texture = WzResource::LoadTexture(frame_node);
  auto origin = WzResource::LoadFpoint(frame_node->get_child(u"origin"));
  auto& camera = CameraGameInstance::camera_;
  SDL_FRect pos_rect{
      pet.pos.x - origin.x,
      pet.pos.y - origin.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  if (!SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    return;
  }
  pos_rect.x -= camera.x;
  pos_rect.y -= camera.y;
  SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect, 0,
                           nullptr,
                           pet.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

// 和 CharacterRenderSystem::RenderNametag 一样：UI/NameTag.img 里每个配色是
// w（左端）/c（可平铺的中间段）/e（右端）三张图拼出来的名签。
void RenderNameTagTexture(wz::Node* texture_node,
                          const std::u16string& text,
                          float text_w,
                          float x,
                          float y,
                          const SDL_FRect& camera) {
  auto* texture_w = WzResource::LoadTexture(texture_node->get_child(u"w"));
  auto* texture_c = WzResource::LoadTexture(texture_node->get_child(u"c"));
  auto* texture_e = WzResource::LoadTexture(texture_node->get_child(u"e"));
  if (texture_w == nullptr || texture_c == nullptr || texture_e == nullptr) {
    return;
  }
  auto ow = WzResource::LoadFpoint(texture_node->find(u"w/origin"));
  auto oc = WzResource::LoadFpoint(texture_node->find(u"c/origin"));

  auto a = static_cast<float>(texture_c->w);
  // 中间段长度取 c 宽度的整数倍，和原客户端一致。
  auto result = a * std::ceil(text_w / a);

  SDL_FRect rect;
  rect.w = result + texture_w->w + texture_e->w;
  rect.h = texture_c->h;
  rect.x = x - rect.w / 2;
  rect.y = y;
  if (!SDL_HasRectIntersectionFloat(&rect, &camera)) {
    return;
  }
  rect.x = static_cast<float>(static_cast<int>(rect.x - camera.x));
  rect.y = static_cast<float>(static_cast<int>(rect.y - camera.y));
  rect.w = static_cast<float>(texture_w->w);
  rect.h = static_cast<float>(texture_w->h);
  SDL_RenderTexture(Window::renderer_, texture_w, nullptr, &rect);
  rect.x += texture_w->w;
  rect.y = static_cast<float>(static_cast<int>(y - camera.y));
  rect.w = result;
  rect.h = static_cast<float>(texture_c->h);
  SDL_RenderTextureTiled(Window::renderer_, texture_c, nullptr, 1, &rect);
  rect.x += result;
  rect.w = static_cast<float>(texture_e->w);
  SDL_RenderTexture(Window::renderer_, texture_e, nullptr, &rect);

  auto clr =
      static_cast<wz::Property<int32_t>*>(texture_node->get_child(u"clr"))
          ->get();
  auto ca = (clr >> 24) & 0xFF;
  auto cr = (clr >> 16) & 0xFF;
  auto cg = (clr >> 8) & 0xFF;
  auto cb = clr & 0xFF;
  Freetype::LoadColor(cr, cg, cb, ca);
  Freetype::DrawLine(text, rect.x - result, y - camera.y);
}

// 画一只宠物：本体 + 宠物装备 + 说话气泡（+ 地面名签）。
bool RenderPet(GamePet& pet, const GameCharacter& owner, bool hanging) {
  auto* pet_node = PetGameInstance::LoadPetNode(pet.id);
  if (pet_node == nullptr) {
    return false;
  }
  auto* action_node = pet_node->get_child(pet.action);
  if (action_node == nullptr) {
    return false;
  }
  auto* frame_node = action_node->get_child(std::to_string(pet.ani_index));
  if (frame_node == nullptr) {
    return false;
  }
  if (frame_node->type == wz::Type::UOL) {
    frame_node = static_cast<wz::Property<wz::WzUOL>*>(frame_node)->get_uol();
  }
  if (frame_node == nullptr || frame_node->get_child(u"origin") == nullptr) {
    return false;
  }

  auto& camera = CameraGameInstance::camera_;
  auto texture = WzResource::LoadTexture(frame_node);
  auto origin = WzResource::LoadFpoint(frame_node->get_child(u"origin"));
  SDL_FRect pos_rect{
      pet.pos.x - origin.x,
      pet.pos.y - origin.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
  if (!SDL_HasRectIntersectionFloat(&pos_rect, &camera)) {
    return false;
  }
  pos_rect.x -= camera.x;
  pos_rect.y -= camera.y;
  SDL_RenderTextureRotated(Window::renderer_, texture, nullptr, &pos_rect, 0,
                            nullptr,
                            pet.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
  DrawPetEquip(pet, owner);
  // 095 CPet::ShowEffect：地图特效（跟丢拉回的 PetEff warp 等）。
  // 动画推进/生命周期在 PetLogicSystem::Run 里，这里只画。
  for (auto& e : pet.effect) {
    EffectRenderSystem::Render(pet.pos, e, pet.flip);
  }
  if (pet.chatballoon.has_value()) {
    // 095 CChatBalloon::MakeBalloon：尾巴尖端贴在宠物头顶上。
    SDL_FPoint balloon_base{
        pet.pos.x,
        pet.pos.y - origin.y - static_cast<float>(texture->h) / 8.0f,
    };
    ChatballoonRenderSystem::Render(pet.chatballoon.value(), balloon_base);
  }
  if (!hanging) {
    PetRenderSystem::RenderNametag(pet);
  }
  return true;
}

}  // namespace

bool PetRenderSystem::Render(uint8_t page, bool hanging) {
  auto& self = CharacterGameInstance::self_;
  for (auto& pet : PetGameInstance::data_) {
    // 095 HangOnBack：挂背上的宠物画在角色之后，地面跟随的宠物画在角色之前。
    if (pet.page != page || pet.hang_back != hanging) {
      continue;
    }
    RenderPet(pet, self, hanging);
  }
  for (auto&& [client_id, pet] : PetGameInstance::others_) {
    auto it = CharacterGameInstance::others_.find(client_id);
    if (it == CharacterGameInstance::others_.end()) {
      continue;
    }
    if (pet.page != page || pet.hang_back != hanging) {
      continue;
    }
    RenderPet(pet, it->second.g_character, hanging);
  }
  return true;
}

void PetRenderSystem::RenderNametag(const GamePet& pet) {
  if (pet.name.empty()) {
    return;
  }
  const auto& camera = CameraGameInstance::camera_;
  Freetype::LoadAligned(true);
  Freetype::LoadSize(13);
  auto w = Freetype::LoadW(pet.name);
  auto h = Freetype::LoadLh();

  // 095 MakeNameTag：名签在宠物脚下（和怪物名签的位置一致）。
  auto x = pet.pos.x;
  auto y = pet.pos.y + 4.0f;

  // 095 CPetTemplate::nNameTag：模板 info/nameTag 是 UI/NameTag.img 的配色号。
  auto index = pet.default_nametag
                   ? -1
                   : PetGameInstance::LoadPetNameTagIndex(pet.id);
  if (index >= 0) {
    auto key_text = std::to_string(index);
    std::u16string key{key_text.begin(), key_text.end()};
    static wz::Node* nametag_node = WzResource::ui_->find(u"NameTag.img");
    auto* texture_node = nametag_node->get_child(key);
    if (texture_node != nullptr) {
      RenderNameTagTexture(texture_node, pet.name, w, x, y, camera);
      return;
    }
  }

  // 没有配色信息时用普通黑底白字（和怪物/NPC 名签一致）。
  SDL_FRect rect;
  rect.w = w + 4;
  rect.h = h + 4;
  rect.x = x - rect.w / 2;
  rect.y = y;
  if (SDL_HasRectIntersectionFloat(&rect, &camera)) {
    rect.x -= camera.x;
    rect.y -= camera.y;
    SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, 178);
    SDL_RenderFillRect(Window::renderer_, &rect);
    Freetype::LoadColor(255, 255, 255, 255);
    Freetype::DrawLine(pet.name, x - camera.x - w / 2, y - camera.y);
  }
}

}  // namespace sdlms
