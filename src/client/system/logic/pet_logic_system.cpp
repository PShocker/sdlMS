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

// 095 CVecCtrlPet::WorkUpdateActive：
//   目标点就是主人坐标（m_nPositionContext = 0 时不偏移），
//   水平差 >= nZMassa(80) 才开始追；远距离宠物（longRange）用 170。
//   速度取主人的移动速度（GetShoeAttr().walkSpeed）。
constexpr float kFollowStart = 80.0f;      // nZMassa
constexpr float kFollowStartLong = 170.0f;  // v48 + 20（longRange）
// 追的时候略快于主人，否则同速永远追不上（095 用随机双倍速度弥补）。
constexpr float kFollowSpeedScale = 1.25f;
constexpr float kMoveForce = 1400.0f;
constexpr float kMaxSpeed = 220.0f;
constexpr float kFriction = 800.0f;
constexpr float kGravity = 2000.0f;
constexpr float kJumpSpeed = -620.0f;  // 起跳速度
constexpr float kJumpHeight = 50.0f;   // 095 rcIn: ±50，主人高出这个距离就起跳
constexpr uint64_t kPickCooldown = 100;   // 连续捡取的间隔 0.1s
// 095 CDropPool::TryPickUpDropByPet 的捡取框：x±25、y-50..y+10。
constexpr float kPickRangeX = 25.0f;
constexpr float kPickRangeYUp = 50.0f;
constexpr float kPickRangeYDown = 10.0f;
// 095 饱食度下降间隔（毫秒）与饥饿阈值（CPet::OnValidateStat）
constexpr uint64_t kFullnessInterval = 30000;
constexpr uint8_t kHungryFullness = 20;
// 095 CVecCtrlPet：宠物与主人距离过远（跟不上，如主人跳下高台/
// 传送门瞬移）时拉回主人身边，并在旧位置播 warp 特效掩饰跳变。
constexpr float kWarpDistX = 300.0f;  // 095 rcOut: ±300 / ±200
constexpr float kWarpDistY = 200.0f;

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
  // 循环动作已经在播就不要重来：以前用 ani_index != 0 判断，而 move 的
  // 第一帧索引正好是 0，导致每帧都重置 ani_time，走路动画卡在第一帧。
  if (loop && pet.action == action) {
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
  // 拉回到主人身后一点，避免直接叠在人物身上。
  pet.pos.x = owner.pos.x + (owner.flip == 1 ? -34.0f : 34.0f);
  pet.hspeed = 0.0f;
  pet.vspeed = 0.0f;
  pet.fh = 0;
  pet.following = false;
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

// 095：远距离宠物（info/longRange）的起步距离更远。
float FollowStartDist(const GamePet& pet) {
  const auto ability = PetGameInstance::LoadPetAbility(pet.id);
  return (ability & PetGameInstance::kAbilityLongRange) != 0
             ? kFollowStartLong
             : kFollowStart;
}

// 跟随速度系数：离得远全速，接近停止距离时减速，避免冲过头再急停。
float FollowSpeedScale(const GamePet& pet, float dx) {
  const float start = FollowStartDist(pet);
  const float stop = start * 0.5f;
  const float dist = std::abs(dx);
  if (dist >= start) {
    return 1.0f;
  }
  const float t = (dist - stop) / std::max(1.0f, start - stop);
  return 0.45f + 0.55f * std::clamp(t, 0.0f, 1.0f);
}

// 095 CVecCtrlPet 的跟随开关 + 滞回：超过起步距离开始追，走到停止距离
// 的一半以内才停，避免在阈值上逐帧抖动。
void UpdateFollowing(GamePet& pet, float dx) {
  const float start = FollowStartDist(pet);
  const float stop = start * 0.5f;
  const float dist = std::abs(dx);
  if (pet.following) {
    if (dist <= stop) {
      pet.following = false;
    }
  } else if (dist >= start) {
    pet.following = true;
  }
}

// 095 CVecCtrlPet::WorkUpdateActive 里贴身时的分离输入：宠物和主人重叠时
// 朝主人朝向的反方向（身后）挪开，避免挂绳/拉回后一直贴在一起。
float SeparateSpeed(const GameCharacter& owner, float dx) {
  constexpr float kBehind = 34.0f;   // 目标：站在主人身后 34px
  constexpr float kTooClose = 60.0f;  // 只在贴身范围内生效
  constexpr float kDead = 6.0f;
  if (std::abs(dx) >= kTooClose) {
    return 0.0f;
  }
  const float desired = owner.flip == 1 ? kBehind : -kBehind;
  const float gap = desired - dx;   // >0 需要往左挪（dx 变大）
  if (std::abs(gap) <= kDead) {
    return 0.0f;
  }
  const float speed = std::max(
      CharacterStatGameInstance::GetHspeedMax() * kFollowSpeedScale, 150.0f);
  return (gap > 0.0f ? -1.0f : 1.0f) * speed * 0.45f;
}

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

void PetLogicSystem::RunPreview(GamePet& pet, const GameCharacter& owner) {
  const float delta_time = Window::delta_time_ / 1000.0f;
  // 095 CVecCtrlPet::WorkUpdateActive 的横向跟随：目标就是主人 x，
  // 距离超过 nZMassa(80)（longRange 170）才起步，速度取主人移速。
  const float dx = owner.pos.x - pet.pos.x;
  const float follow_speed = std::max(
      CharacterStatGameInstance::GetHspeedMax() * kFollowSpeedScale, 150.0f);

  UpdateFollowing(pet, dx);
  if (pet.following) {
    const float speed = follow_speed * FollowSpeedScale(pet, dx);
    pet.flip = dx > 0.0f;
    pet.hspeed = (dx > 0.0f ? 1.0f : -1.0f) * speed;
  } else {
    pet.hspeed = SeparateSpeed(owner, dx);
    if (pet.hspeed != 0.0f) {
      pet.flip = pet.hspeed > 0.0f;
    }
  }
  // 这一帧的位移不要越过停止距离，避免「冲过头再倒回来」的抖动。
  float step = pet.hspeed * delta_time;
  if (std::abs(step) > std::abs(dx)) {
    step = dx;
  }
  pet.pos.x += step;
  // 预览场景没有地形数据，直接贴着主人的脚底站。
  pet.pos.y = owner.pos.y;
  pet.fall = false;
  pet.fh = 1;
  pet.hang_back = false;
  pet.vspeed = 0.0f;

  // 动作选择和地图宠物完全一致（095 预览宠物也走 CPet::Update）。
  RunStateMachine(pet, owner);
  RunAnimate(pet);
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
    // 095 HangOnBack：宠物图层 origin 直接对齐主人的 body origin（同一个
    // 角色锚点）。WZ 里 hang 帧的 origin 是 (13,60)、stand0 只有 (19,39)，
    // 也就是说换算到我们「pet.pos = 脚下锚点」的坐标系里，pos 直接用
    // 主人坐标即可，60 的 origin 会把贴图抬到背上。
    pet.pos = owner.pos;
    // 多图层地图：和主人同页，否则会被本页的物件/前景盖住（看起来消失）。
    pet.page = owner.page;
    return;
  }
  if (pet.hang_back) {
    // 人物离开绳子：宠物立刻跟着一起动作（不再延迟 200ms，和人物同时跳）。
    pet.hang_back = false;
    pet.page = owner.page;
    pet.fall = false;
    pet.fh = 0;
    pet.hspeed = 0.0f;
    pet.vspeed = 0.0f;

    const auto& input = CharacterLogicSystem::character_action_input_;
    const int jump_dir = input.contains("left")    ? -1
                         : input.contains("right") ? 1
                         : (owner.flip == 1 ? 1 : -1);
    const bool owner_air =
        owner.action == u"jump" || owner.action == u"fly" ||
        owner.action == u"fall" ||
        (is_self && CharacterLogicSystem::self_fh_ == 0);
    if (owner_air) {
      // 095 m_bJumpNext：和人物同时蹬离绳子，带向上的初速 + 人物方向初速。
      pet.fall = true;
      pet.vspeed = kJumpSpeed;
      pet.hspeed = static_cast<float>(jump_dir) *
                   std::max(CharacterStatGameInstance::GetHspeedMax(), 100.0f);
      pet.flip = jump_dir > 0;
      return;
    }
    // 人物已经踩到平台上：直接贴到身后的落脚点。
    pet.pos = owner.pos;
    pet.pos.x = owner.pos.x + (owner.flip == 1 ? -34.0f : 34.0f);
    if (is_self) {
      pet.fh = CharacterLogicSystem::self_fh_;
    }
    pet.fall = pet.fh == 0;
    pet.following = false;
    return;
  }

  // 095：目标点 = 主人 x；超过阈值才起步，速度用主人的移动速度。
  // 起步/停止用滞回，否则在阈值附近会一帧 move 一帧 stand，看起来抽搐。
  const float dx = owner.pos.x - pet.pos.x;
  // 主人在上方且水平距离不远才跳（否则主人在够不到的高台上时宠物会原地弹）。
  const bool owner_above =
      owner.pos.y < pet.pos.y - kJumpHeight &&
      std::abs(owner.pos.x - pet.pos.x) < 120.0f;
  const float follow_speed = std::max(
      CharacterStatGameInstance::GetHspeedMax() * kFollowSpeedScale, 150.0f);

  UpdateFollowing(pet, dx);
  float hspeed = 0.0f;
  if (pet.following) {
    const float speed = follow_speed * FollowSpeedScale(pet, dx);
    hspeed = dx > 0.0f ? speed : -speed;
    pet.flip = dx > 0.0f;
  } else {
    // 贴在一起时往主人身后退开（095 的 SetInput 分离）。
    hspeed = SeparateSpeed(owner, dx);
    if (hspeed != 0.0f) {
      pet.flip = hspeed > 0.0f;
    }
  }

  auto border = LoadBorder();

  // 空中：主人在上方时用跳跃追上，其余时间保持重力下落。
  if (pet.fh == 0 || pet.fall) {
    // 095 CVecCtrl 空中：保留起跳时的水平初速度（跳离绳子/跳台阶），
    // 输入只做很弱的修正（CUserLocal::RunFall 的 0.1 px/ms² 量级），
    // 之前每帧直接把 hspeed 覆盖成跟随速度，起跳的横向速度会被瞬间清掉。
    const float air_input =
        hspeed > 0.0f ? 1.0f : (hspeed < 0.0f ? -1.0f : 0.0f);
    pet.hspeed += air_input * 0.1f * Window::delta_time_;
    const float air_max = std::max(
        CharacterStatGameInstance::GetHspeedMax() * kFollowSpeedScale, 150.0f);
    pet.hspeed = std::clamp(pet.hspeed, -air_max, air_max);
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

  // 地面：主人跳上更高的台阶时宠物也跳一下（带冷却，避免连续弹跳）。
  if (owner_above && Window::dt_now_ >= pet.next_jump) {
    pet.fall = true;
    pet.vspeed = kJumpSpeed;
    pet.fh = 0;
    pet.next_jump = Window::dt_now_ + 800;
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
  // 095 CDropPool::TryPickUpDropByPet：矩形固定 x±25、y-50..y+10，
  // 和 longRange 无关（longRange 只影响跟随时起步距离）。
  constexpr float range_x = kPickRangeX;
  constexpr float range_up = kPickRangeYUp;
  constexpr float range_down = kPickRangeYDown;
  const uint64_t cooldown = (ability & PetGameInstance::kAbilitySweepForDrop) != 0
                                ? 80
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
  pet.next_pick = Window::dt_now_ + 40;
}

void PetLogicSystem::RunStateMachine(GamePet& pet,
                                     const GameCharacter& owner) {
  // 095 CPet::Update：用主人的 GetPos/GetPosPrev 判断主人有没有动，
  // 这一步和宠物当前动作无关，每帧都要更新。
  const bool owner_moved =
      !pet.owner_prev_valid || owner.pos.x != pet.owner_prev_x ||
      owner.pos.y != pet.owner_prev_y;
  pet.owner_prev_x = owner.pos.x;
  pet.owner_prev_y = owner.pos.y;
  pet.owner_prev_valid = true;
  if (owner_moved) {
    pet.t_stand = 0;
    pet.random_action_done = false;
  } else {
    pet.t_stand += static_cast<uint32_t>(Window::delta_time_);
  }

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
  // 095：还没走到跟随范围内（>= nZMassa）就继续播放 move。
  const bool needs_move = pet.following;

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
    return;
  }

  // 095：m_tStand > 120000（2 分钟）→ DoActionByUserAction(6) = sleep。
  if (pet.t_stand > 120000) {
    auto action = PickAction(pet, {u"sleep", u"rest0", u"stand0"});
    if (!action.empty() && action != pet.action) {
      EnterAction(pet, action, false);
      pet.t_stand = 0;
      return;
    }
  }
  // 095：m_tStand > 5000 后每 5000ms 窗口掷一次（50% 概率）播随机动作。
  if (pet.t_stand > 5000 && pet.t_stand % 5000 < 30) {
    if (!pet.random_action_done) {
      pet.random_action_done = true;
      std::uniform_int_distribution<int> coin(0, 1);
      if (coin(RandomGameInstance::gen_) == 1 && RandomIdleAction(pet)) {
        return;
      }
    }
  } else if (pet.t_stand % 5000 >= 30) {
    pet.random_action_done = false;
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
