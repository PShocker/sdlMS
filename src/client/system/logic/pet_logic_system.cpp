#include "src/client/system/logic/pet_logic_system.h"

#include "src/client/game/game_drop.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/pet_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace sdlms {

namespace {

// 095 CPet::Update：宠物跟着主人跑，落后主人一小段距离。
constexpr float kFollowGap = 30.0f;  // 与主人保持的水平距离
constexpr float kFollowDead = 8.0f;  // 小于这个距离就停下
constexpr float kMoveForce = 1400.0f;
constexpr float kMaxSpeed = 220.0f;
constexpr float kFriction = 800.0f;
constexpr float kGravity = 2000.0f;
constexpr float kJumpSpeed = -620.0f;  // 起跳速度
constexpr float kJumpHeight = 40.0f;   // 主人在上方多少像素时起跳
constexpr uint64_t kPickCooldown = 500;
// 095 CDropPool::TryPickUpDropByPet 的捡取框：x±25、y-50..y+10。
constexpr float kPickRangeX = 25.0f;
constexpr float kPickRangeYUp = 50.0f;
constexpr float kPickRangeYDown = 10.0f;
// 095 饱食度下降间隔（毫秒）与饥饿阈值（CPet::OnValidateStat）
constexpr uint64_t kFullnessInterval = 30000;
constexpr uint8_t kHungryFullness = 20;
// 095 CVecCtrlPet：宠物与主人距离过远（跟不上，如主人跳下高台/
// 传送门瞬移）时拉回主人身边，并在旧位置播 warp 特效掩饰跳变。
constexpr float kWarpDistX = 220.0f;
constexpr float kWarpDistY = 160.0f;

bool IsFrameKey(const std::u16string& key) {
  return !key.empty() && std::ranges::all_of(key, [](char16_t c) {
    return c >= u'0' && c <= u'9';
  });
}

std::vector<std::u16string> LoadFrameKeys(wz::Node* action) {
  std::vector<std::u16string> keys;
  if (action == nullptr || action->get_children() == nullptr) {
    return keys;
  }
  for (const auto& [key, value] : *action->get_children()) {
    if (!value.empty() && value[0] != nullptr && IsFrameKey(key)) {
      keys.push_back(key);
    }
  }
  std::ranges::sort(keys, [](const std::u16string& a, const std::u16string& b) {
    return std::stoul(std::string{a.begin(), a.end()}) <
           std::stoul(std::string{b.begin(), b.end()});
  });
  return keys;
}

uint32_t FrameNumber(const std::u16string& key) {
  return static_cast<uint32_t>(std::stoul(std::string{key.begin(), key.end()}));
}

uint32_t FirstFrameIndex(wz::Node* action) {
  auto keys = LoadFrameKeys(action);
  return keys.empty() ? 0 : FrameNumber(keys.front());
}

uint32_t FrameDelay(wz::Node* action, const std::u16string& key) {
  auto* frame = action == nullptr ? nullptr : action->get_child(key);
  if (frame == nullptr) {
    return 100;
  }
  auto* delay = frame->get_child(u"delay");
  if (delay == nullptr && frame->type == wz::Type::UOL) {
    if (auto* real = static_cast<wz::Property<wz::WzUOL>*>(frame)->get_uol();
        real != nullptr) {
      delay = real->get_child(u"delay");
    }
  }
  if (delay == nullptr || delay->type != wz::Type::Int) {
    return 100;
  }
  return static_cast<uint32_t>(
      static_cast<wz::Property<int32_t>*>(delay)->get());
}

wz::Node* LoadActionNode(const GamePet& pet, const std::u16string& action) {
  auto* pet_node = PetGameInstance::LoadPetNode(pet.id);
  if (pet_node == nullptr) {
    return nullptr;
  }
  return pet_node->get_child(action);
}

// 按优先级取第一个有帧的动作名。
std::u16string PickAction(const GamePet& pet,
                          std::initializer_list<const char16_t*> names) {
  for (const auto* name : names) {
    auto* node = LoadActionNode(pet, name);
    if (node != nullptr && !LoadFrameKeys(node).empty()) {
      return name;
    }
  }
  return u"";
}

void EnterAction(GamePet& pet, const std::u16string& action, bool loop) {
  auto* node = LoadActionNode(pet, action);
  if (pet.action == action && pet.ani_index != 0) {
    return;
  }
  pet.action = action;
  pet.ani_index = FirstFrameIndex(node);
  pet.ani_time = 0;
  if (loop) {
    pet.action_end = 0;
  } else {
    uint32_t cycle = 0;
    for (const auto& key : LoadFrameKeys(node)) {
      cycle += FrameDelay(node, key);
    }
    pet.action_end = Window::dt_now_ + std::max<uint32_t>(cycle, 100);
  }
}

// 095 RandomAction：站立一段时间后随机播一个 idle 小动作，播完自动回 stand。
bool RandomIdleAction(GamePet& pet) {
  // 饿了就优先播饥饿动作。
  if (pet.fullness <= kHungryFullness) {
    std::uniform_int_distribution<int> hungry_chance(0, 99);
    if (hungry_chance(RandomGameInstance::gen_) < 60) {
      auto action = PickAction(pet, {u"hungry", u"cry", u"angry"});
      if (!action.empty()) {
        EnterAction(pet, action, false);
        pet.next_idle = Window::dt_now_ + 4000;
        return true;
      }
    }
  }
  static constexpr std::u16string_view kIdleActions[] = {
      u"rest0", u"rest1", u"nap",   u"tedious", u"stretch",
      u"slang", u"chat",  u"angry", u"cry",     u"food",
  };
  std::uniform_int_distribution<int> chance(0, 99);
  if (chance(RandomGameInstance::gen_) >= 35) {
    return false;
  }
  std::vector<std::u16string> candidates;
  for (auto name : kIdleActions) {
    std::u16string action{name};
    auto* node = LoadActionNode(pet, action);
    if (node != nullptr && !LoadFrameKeys(node).empty()) {
      candidates.push_back(std::move(action));
    }
  }
  if (candidates.empty()) {
    return false;
  }
  std::uniform_int_distribution<size_t> pick(0, candidates.size() - 1);
  EnterAction(pet, candidates[pick(RandomGameInstance::gen_)], false);
  pet.next_idle = Window::dt_now_ + 4000;
  return true;
}

// 在宠物脚下找一块 foothold 并落上去（下绳子时避免直接从地面穿下去）。
bool SnapToGround(GamePet& pet) {
  auto& fhs = FootholdGameInstance::data_;
  if (fhs.empty()) {
    return false;
  }
  const SDL_FPoint from{pet.pos.x, pet.pos.y - 4.0f};
  const SDL_FPoint to{pet.pos.x, pet.pos.y + 400.0f};
  auto inter = Physic::FallIntersectPos(from, to, fhs);
  for (const auto& [y, hit] : inter) {
    if (!hit.fh.k.has_value()) {
      continue;
    }
    pet.pos = hit.pos;
    pet.fh = hit.fh.id;
    pet.page = hit.fh.page;
    pet.fall = false;
    pet.vspeed = 0.0f;
    pet.hspeed = 0.0f;
    return true;
  }
  return false;
}

// 095 CPet::ShowEffect(2)：跟丢拉回时在旧位置播 Effect/PetEff.img/
// <7位宠物id>/warp（一个由星星组成的消散动画），随后宠物瞬移到主人
// 身边落地。数据包没有该宠物的 warp 贴图时只瞬移不播特效。
// warp 会广播给同地图其他玩家（远端在旧位置播同款特效）。
void WarpToOwner(GamePet& pet, const GameCharacter& owner) {
  const SDL_FPoint old_pos = pet.pos;
  auto key = PetGameInstance::LoadPetImageKey(pet.id);
  if (!key.empty()) {
    auto path = u"PetEff.img/" + key + u"/warp";
    // 找不到节点时 effect_->find 返回 nullptr，判空避免后续解引用。
    if (WzResource::effect_->find(path) != nullptr) {
      GameEffect e{};
      e.id = path;
      e.type = GameEffect::EffectType::kEffect;
      e.lv = static_cast<uint8_t>(GameEffect::EffectSubType::kEffect);
      // 渲染以 pet.pos（拉回后的新位置）为基准，旧位置 = 新位置 + 偏移。
      e.pos = SDL_FPoint{old_pos.x - owner.pos.x,
                         old_pos.y - owner.pos.y - 10.0f};
      e.z = false;
      e.flip = pet.flip;
      e.delay = 0;
      pet.effect.push_back(std::move(e));
    }
  }
  pet.pos = owner.pos;
  pet.hspeed = 0.0f;
  pet.vspeed = 0.0f;
  pet.fh = 0;
  if (!SnapToGround(pet)) {
    pet.fall = true;
  }
  // 广播 warp：action="warp"，x/y = 旧位置（远端在旧位置播特效）。
  fbs::ClientCharacterPetActionT packet;
  packet.map_id = SceneSystemInstance::map_id_;
  packet.action = "warp";
  packet.x = old_pos.x;
  packet.y = old_pos.y;
  ClientRequest::SendToHost(packet);
}

std::optional<SDL_FRect> LoadBorder() {
  auto map_id = SceneSystemInstance::map_id_;
  auto border = MapInfoGameInstance::LoadMrBorder(map_id);
  if (border.w <= border.x) {
    return std::nullopt;
  }
  return border;
}

}  // namespace

void PetLogicSystem::RunAnimate(GamePet& pet) {
  auto* action_node = LoadActionNode(pet, pet.action);
  auto keys = LoadFrameKeys(action_node);
  if (keys.empty()) {
    return;
  }

  auto current_key = std::to_string(pet.ani_index);
  auto it = std::ranges::find(
      keys, std::u16string{current_key.begin(), current_key.end()});
  if (it == keys.end()) {
    pet.ani_index = FirstFrameIndex(action_node);
    it = keys.begin();
  }

  pet.ani_time += Window::delta_time_;
  if (pet.ani_time < FrameDelay(action_node, *it)) {
    return;
  }
  pet.ani_time = 0;
  ++it;
  if (it == keys.end()) {
    pet.ani_index = FirstFrameIndex(action_node);
    return;
  }
  pet.ani_index = FrameNumber(*it);
}

void PetLogicSystem::RunFollow(GamePet& pet, const GameCharacter& owner,
                               bool is_self) {
  const float delta_time = Window::delta_time_ / 1000.0f;

  // 095 CVecCtrlPet 跟丢拉回：距离过远时瞬移到主人身边并播 warp。
  // 挂背（上绳）不算跟丢；只在正常跟随阶段检测。
  if (!pet.hang_back &&
      (std::abs(pet.pos.x - owner.pos.x) > kWarpDistX ||
       std::abs(pet.pos.y - owner.pos.y) > kWarpDistY)) {
    WarpToOwner(pet, owner);
    return;
  }

  // 095 HangOnBack：主人爬上绳子/梯子时，宠物挂在主人背上，跟着一起升降。
  const bool owner_on_rope =
      is_self ? (CharacterLogicSystem::self_lr_ != 0)
              : (owner.action == u"ladder" || owner.action == u"rope");
  if (owner_on_rope) {
    pet.hang_back = true;
    pet.fall = false;
    pet.fh = 0;
    pet.hspeed = 0.0f;
    pet.vspeed = 0.0f;
    pet.flip = owner.flip == 1;
    pet.pos = owner.pos;
    return;
  }
  if (pet.hang_back) {
    // 主人离开绳子：从主人身上松手，落到地面后继续跟随。
    pet.hang_back = false;
    pet.pos = owner.pos;
    pet.hspeed = 0.0f;
    pet.vspeed = 0.0f;
    if (!SnapToGround(pet)) {
      pet.fh = 0;
      pet.fall = true;
    }
    return;
  }

  const bool owner_right = owner.flip == 1;
  // 主人朝右时宠物在左侧（身后），反之亦然。
  const float target_x = owner.pos.x + (owner_right ? -kFollowGap : kFollowGap);
  const float dx = target_x - pet.pos.x;
  const bool owner_above = owner.pos.y < pet.pos.y - kJumpHeight;

  float hspeed = 0.0f;
  if (std::abs(dx) > kFollowDead) {
    hspeed = dx > 0.0f ? kMaxSpeed : -kMaxSpeed;
    pet.flip = dx > 0.0f;
  }

  auto border = LoadBorder();

  // 空中：主人在上方时用跳跃追上，其余时间保持重力下落。
  if (pet.fh == 0 || pet.fall) {
    pet.hspeed = hspeed;
    pet.vspeed += kGravity * delta_time;
    auto& fhs = FootholdGameInstance::data_;
    const bool airborne =
        Physic::Fall(pet.pos, delta_time, pet.hspeed, pet.vspeed, -2000.0f,
                     2000.0f, border, true, true, pet.fh, pet.page, fhs);
    pet.fall = airborne;
    if (!airborne) {
      pet.vspeed = 0.0f;
      return;
    }
    // 掉出地图底边（例如主人从绳子上跳下去）：拉回主人身边，
    // 旧位置播 warp（095 ShowEffect(2)）。
    if (border.has_value() && pet.pos.y > border->h) {
      WarpToOwner(pet, owner);
    }
    return;
  }

  // 地面：主人跳上更高的台阶时宠物也跳一下。
  if (owner_above) {
    pet.fall = true;
    pet.vspeed = kJumpSpeed;
    pet.fh = 0;
    return;
  }

  auto& fhs = FootholdGameInstance::data_;
  if (!fhs.contains(pet.fh)) {
    pet.fh = 0;
    pet.fall = true;
    return;
  }
  pet.hspeed = hspeed;
  pet.hforce =
      hspeed > 0.0f ? kMoveForce : (hspeed < 0.0f ? -kMoveForce : 0.0f);
  const bool walking =
      Physic::Walk(pet.pos, delta_time, pet.hspeed, pet.vspeed, pet.hforce,
                   -kMaxSpeed, kMaxSpeed, kFriction, true, pet.fh, border, fhs);
  if (!walking) {
    // 走到边缘掉下去，交给下落分支处理。
    pet.fall = true;
  }
}

void PetLogicSystem::RunPick(GamePet& pet) {
  if (Window::dt_now_ < pet.next_pick) {
    return;
  }
  // 空中不捡东西（095 里宠物在落地后才拾取）；挂在主人背上、或饿到 0 时也不捡。
  if (pet.hang_back || pet.fh == 0 || pet.fall || pet.fullness == 0) {
    return;
  }
  // 095 CDropPool::TryPickUpDropByPet：金币要 pickupMeso，道具要 pickupItem 且
  // 不在 ignoreItems；longRange 扩大范围，sweepForDrop 缩短节流。
  const auto ability = PetGameInstance::LoadPetAbility(pet.id);
  const bool allow_item =
      (ability & PetGameInstance::kAbilityPickupItem) != 0 &&
      (ability & PetGameInstance::kAbilityIgnoreItems) == 0;
  const bool allow_meso = (ability & PetGameInstance::kAbilityPickupMeso) != 0;
  if (!allow_item && !allow_meso) {
    return;
  }
  const bool long_range = (ability & PetGameInstance::kAbilityLongRange) != 0;
  const float range_x = long_range ? 75.0f : kPickRangeX;
  const float range_up = long_range ? 90.0f : kPickRangeYUp;
  const float range_down = long_range ? 30.0f : kPickRangeYDown;
  const uint64_t cooldown = (ability & PetGameInstance::kAbilitySweepForDrop) != 0
                                ? 250
                                : kPickCooldown;
  for (auto& drop : DropGameInstance::data_ | std::views::values) {
    if (drop.type != GameDrop::DropEnum::kLand) {
      continue;
    }
    const bool is_meso = drop.data->id == u"00000000";
    if (is_meso ? !allow_meso : !allow_item) {
      continue;
    }
    if (PackageGameInstance::LoadEmptyIndex(drop.data).empty()) {
      continue;
    }
    const float dx = drop.goal.x - pet.pos.x;
    const float dy = drop.goal.y - pet.pos.y;
    if (std::abs(dx) > range_x || dy < -range_up || dy > range_down) {
      continue;
    }
    pet.next_pick = Window::dt_now_ + cooldown;
    fbs::ClientCharacterPickT pick;
    pick.map_id = SceneSystemInstance::map_id_;
    pick.random_id = drop.random_id;
    pick.pet_index = pet.index;
    ClientRequest::SendToHost(pick);
    return;
  }
  pet.next_pick = Window::dt_now_ + 100;
}

void PetLogicSystem::RunStateMachine(GamePet& pet,
                                     const GameCharacter& owner) {
  // 095 HangOnBack：挂在主人背上时只播 hang。
  if (pet.hang_back) {
    auto action = PickAction(pet, {u"hang", u"prone", u"stand0"});
    if (!action.empty()) {
      EnterAction(pet, action, true);
    }
    return;
  }

  // 一次性动作（idle 表情等）播完后回到站立。
  if (pet.action_end != 0) {
    if (Window::dt_now_ < pet.action_end) {
      return;
    }
    pet.action_end = 0;
    auto action = PickAction(pet, {u"stand0", u"stand1"});
    if (!action.empty()) {
      EnterAction(pet, action, true);
    }
    pet.next_idle = Window::dt_now_ + 4000;
    return;
  }

  const bool airborne = pet.fh == 0 || pet.fall;
  const bool moving = std::abs(pet.hspeed) > 1.0f;
  const float target_x =
      owner.pos.x + (owner.flip == 1 ? -kFollowGap : kFollowGap);
  const bool needs_move = std::abs(target_x - pet.pos.x) > kFollowDead;

  if (airborne) {
    // 095：上升用 jump，下落用 fly（有 fly 动作的宠物）。
    auto action = pet.vspeed < 0.0f
                      ? PickAction(pet, {u"jump", u"fly", u"move"})
                      : PickAction(pet, {u"fly", u"jump", u"move"});
    if (!action.empty()) {
      EnterAction(pet, action, true);
    }
    return;
  }
  if (moving || needs_move) {
    auto action = PickAction(pet, {u"move", u"stand0"});
    if (!action.empty()) {
      EnterAction(pet, action, true);
    }
    return;
  }
  // 站立：stand0/stand1 交替，偶尔播一次 idle 小动作（095 RandomAction）。
  if (pet.action != u"stand0" && pet.action != u"stand1") {
    auto action = PickAction(pet, {u"stand0", u"stand1"});
    if (!action.empty()) {
      EnterAction(pet, action, true);
    }
    pet.next_idle = Window::dt_now_ + 4000;
    return;
  }
  if (Window::dt_now_ >= pet.next_idle && pet.next_idle != 0) {
    if (RandomIdleAction(pet)) {
      return;
    }
    const std::u16string other =
        pet.action == u"stand0" ? u"stand1" : u"stand0";
    if (!PickAction(pet, {other.c_str()}).empty()) {
      EnterAction(pet, other, true);
    }
    pet.next_idle = Window::dt_now_ + 4000;
  }
}

bool PetLogicSystem::Run() {
  // 背包里的宠物道具可能变化（捡到/丢掉），定期刷新列表。
  static uint64_t next_refresh = 0;
  // 宠物装备穿/脱后要把新 equip_id 重新上报（随宠物消息同步）。
  static uint32_t last_equip_id = 0;
  if (PetGameInstance::data_.empty() || Window::dt_now_ >= next_refresh) {
    PetGameInstance::Refresh();
    next_refresh = Window::dt_now_ + 1000;
  }
  if (!PetGameInstance::data_.empty()) {
    uint32_t equip_id = 0;
    if (const auto& self = CharacterGameInstance::self_;
        self.pet_equip.has_value()) {
      try {
        equip_id = static_cast<uint32_t>(std::stoi(
            std::string{self.pet_equip->id.begin(),
                        self.pet_equip->id.end()}));
      } catch (...) {
        equip_id = 0;
      }
    }
    if (equip_id != last_equip_id) {
      last_equip_id = equip_id;
      PetGameInstance::SendToServer();
    }
  } else {
    last_equip_id = 0;
  }

  // 095 CPet::OnValidateStat：饱食度随时间下降，0 时宠物不再捡取。
  static uint64_t next_fullness = 0;
  if (!PetGameInstance::data_.empty() && Window::dt_now_ >= next_fullness) {
    next_fullness = Window::dt_now_ + kFullnessInterval;
    for (auto& pet : PetGameInstance::data_) {
      if (pet.fullness > 0) {
        --pet.fullness;
        PetGameInstance::SetFullness(pet.fullness);
      }
    }
  }

  // 095 宠物装备能力 consumeHP/consumeMP：主人 HP/MP 低于一半时自动吃药。
  static uint64_t next_potion = 0;
  if (!PetGameInstance::data_.empty() && Window::dt_now_ >= next_potion) {
    const auto ability = PetGameInstance::LoadPetAbility(PetGameInstance::data_.front().id);
    const int32_t hp = CharacterStatGameInstance::GetHp();
    const int32_t hp_max = CharacterStatGameInstance::GetHpMax();
    const int32_t mp = CharacterStatGameInstance::GetMp();
    const int32_t mp_max = CharacterStatGameInstance::GetMpMax();
    const bool need_hp =
        (ability & PetGameInstance::kAbilityConsumeHP) != 0 && hp > 0 &&
        hp_max > 0 && hp * 2 < hp_max;
    const bool need_mp =
        (ability & PetGameInstance::kAbilityConsumeMP) != 0 && mp > 0 &&
        mp_max > 0 && mp * 2 < mp_max;
    const bool used =
        (need_hp || need_mp) &&
        PetGameInstance::UseAutoPotion(need_hp, need_mp);
    if (used) {
      next_potion = Window::dt_now_ + 2000;
      if (need_hp) {
        // 095 CUserLocal::TryConsumePetHP：自动喝下 HP 药时宠物提醒
        //（noHPAlert）。
        PetGameInstance::AutoSpeakEvent(3);
      }
    } else if (need_hp || need_mp) {
      // 无药可喝：对应 095 的 noNOHPpotion/noNOMPpotion 事件，
      // 30 秒提醒一次避免刷屏。
      next_potion = Window::dt_now_ + 30000;
      if (need_hp) {
        PetGameInstance::AutoSpeakEvent(4);
      }
      if (need_mp) {
        PetGameInstance::AutoSpeakEvent(5);
      }
    }
  }

  // 095 CPet::Update：宠物在场每 30 分钟随机说一句（m_tAutoSpeakingTimer
  // 累计 1800000ms；台词数据缺失时静默）。
  static uint64_t next_auto_speak = 0;
  if (!PetGameInstance::data_.empty()) {
    if (next_auto_speak == 0) {
      next_auto_speak = Window::dt_now_ + 1800000;
    } else if (Window::dt_now_ >= next_auto_speak) {
      PetGameInstance::AutoSpeakRandom();
      next_auto_speak = Window::dt_now_ + 1800000;
    }
  }

  // 095 CPet 的移动包：每 300ms 把自己的宠物位置/当前帧同步给其他玩家。
  static uint64_t next_move = 0;
  if (!PetGameInstance::data_.empty() && Window::dt_now_ >= next_move) {
    PetGameInstance::SendMoveToServer();
    next_move = Window::dt_now_ + 300;
  }

  auto& self = CharacterGameInstance::self_;
  for (auto& pet : PetGameInstance::data_) {
    if (self.tomb.has_value()) {
      continue;  // 主人死亡时不跟随
    }
    RunFollow(pet, self, true);
    RunStateMachine(pet, self);
    RunAnimate(pet);
    RunPick(pet);
  }

  // 其他玩家的宠物：只跟随主人移动/播动作，不捡东西。
  for (auto&& [client_id, pet] : PetGameInstance::others_) {
    auto it = CharacterGameInstance::others_.find(client_id);
    if (it == CharacterGameInstance::others_.end()) {
      continue;
    }
    const auto& owner = it->second.g_character;
    RunFollow(pet, owner, false);
    RunStateMachine(pet, owner);
    RunAnimate(pet);
  }
  return true;
}

}  // namespace sdlms
