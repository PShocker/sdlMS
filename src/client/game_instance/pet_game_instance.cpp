#include "src/client/game_instance/pet_game_instance.h"

#include "src/client/game/game_item.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"

#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <flat_map>
#include <flat_set>
#include <format>
#include <optional>
#include <random>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace sdlms {

namespace {

  // 道具 id 是 8 位（05000000），而 Item.wz 里的宠物图片是 7 位（5000000.img）。
std::u16string PetImageKey(const std::u16string& id) {
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  return std::u16string(view.begin(), view.end());
}

// String.wz/Pet.img 的子节点就是数据里实际存在的宠物模板 id；用它来判断
// 一个道�?id 是不是宠物，避免对不存在的宠物图片调�?wz::Files::find（会
// assert）�?
const std::flat_set<std::u16string>& PetTemplateKeys() {
  static const std::flat_set<std::u16string> keys = [] {
    std::flat_set<std::u16string> r;
    auto* image = WzResource::string_->find(u"Pet.img");
    if (image != nullptr && image->get_children() != nullptr) {
      for (const auto& [key, value] : *image->get_children()) {
        r.insert(key);
      }
    }
    return r;
  }();
  return keys;
}

}  // namespace

bool PetGameInstance::IsPetItem(const std::u16string& id) {
  auto str = id;
  if (str.size() < 8) {
    str.insert(0, 8 - str.size(), u'0');
  }
  // 宠物道具是 05000000 起的现金道具，对应 Item.wz/Pet/<7 位 id>.img。
  if (!str.starts_with(u"0500")) {
    return false;
  }
  return PetTemplateKeys().contains(PetImageKey(str));
}

wz::Node* PetGameInstance::LoadPetNode(const std::u16string& id) {
  static std::flat_map<std::u16string, wz::Node*> cache;
  if (auto it = cache.find(id); it != cache.end()) {
    return it->second;
  }
  wz::Node* node = nullptr;
  if (IsPetItem(id)) {
    node = WzResource::item_->find(u"Pet/" + PetImageKey(id) + u".img");
  }
  cache.emplace(id, node);
  return node;
}

std::u16string PetGameInstance::LoadPetImageKey(const std::u16string& id) {
  auto str = id;
  if (str.size() < 8) {
    str.insert(0, 8 - str.size(), u'0');
  }
  if (!str.starts_with(u"0500")) {
    return u"";
  }
  return PetImageKey(str);
}

std::u16string PetGameInstance::LoadPetName(const std::u16string& id) {
  auto key = PetImageKey(id);
  if (key.empty()) {
    return u"";
  }
  auto* image = WzResource::string_->find(u"Pet.img");
  if (image == nullptr) {
    return u"";
  }
  auto* entry = image->get_child(key);
  if (entry == nullptr) {
    return u"";
  }
  auto* name_node = entry->get_child(u"name");
  if (name_node == nullptr || name_node->type != wz::Type::String) {
    return u"";
  }
  return static_cast<wz::Property<std::u16string>*>(name_node)->get();
}

GamePet* PetGameInstance::FindPet(int32_t index) {
  for (auto& pet : data_) {
    if (pet.index == index) {
      return &pet;
    }
  }
  return nullptr;
}

int32_t PetGameInstance::LoadPetNameTagIndex(const std::u16string& id) {
  auto* node = LoadPetNode(id);
  if (node == nullptr) {
    return -1;
  }
  auto* info = node->get_child(u"info");
  if (info == nullptr) {
    return -1;
  }
  // 095 CPetTemplate::Load 读的是 info 的 nameTag/nametag 整型值；
  // 数据里两种拼写都出现过，先读 nameTag 再退回 nametag。
  for (auto key : {u"nameTag", u"nametag"}) {
    auto* tag = info->get_child(key);
    if (tag != nullptr && tag->type == wz::Type::Int) {
      return static_cast<wz::Property<int32_t>*>(tag)->get();
    }
  }
  return -1;
}

bool PetGameInstance::LoadPetCanPickItem(const std::u16string& id) {
  auto* node = LoadPetNode(id);
  if (node == nullptr) {
    return false;
  }
  auto* info = node->get_child(u"info");
  if (info == nullptr) {
    // 没有 info 的宠物按 095 的默认值处理（能捡）。
    return true;
  }
  auto* pick = info->get_child(u"pickupItem");
  if (pick == nullptr || pick->type != wz::Type::Int) {
    return true;
  }
  return static_cast<wz::Property<int32_t>*>(pick)->get() != 0;
}

void PetGameInstance::Clear() {
  data_.clear();
  others_.clear();
}

void PetGameInstance::SendToServer() {
  SyncActiveState();
  fbs::ClientCharacterPetT packet;
  packet.map_id = SceneSystemInstance::map_id_;
  packet.pet = std::make_unique<fbs::PetSaveT>();
  if (active_ && !pet_id_.empty()) {
    if (const auto* state = ActivePet(); state != nullptr) {
      packet.pet->item_id = state->item_id;
      packet.pet->name = {state->name.begin(), state->name.end()};
      packet.pet->level = state->level;
      packet.pet->fullness = state->fullness;
      packet.pet->closeness = state->closeness;
      packet.pet->active = true;
    }
  } else {
    packet.pet->item_id = 0;
    packet.pet->active = false;
  }
  ClientRequest::SendToHost(packet);
}

void PetGameInstance::SendActionToServer(const std::u16string& action,
                                        const std::u16string& chat) {
  fbs::ClientCharacterPetActionT packet;
  packet.map_id = SceneSystemInstance::map_id_;
  packet.action = {action.begin(), action.end()};
  packet.chat = {chat.begin(), chat.end()};
  ClientRequest::SendToHost(packet);
}

void PetGameInstance::SendMoveToServer() {
  if (!active_ || data_.empty()) {
    return;
  }
  const auto& pet = data_.front();
  fbs::ClientCharacterPetMoveT move;
  move.map_id = SceneSystemInstance::map_id_;
  move.x = pet.pos.x;
  move.y = pet.pos.y;
  move.page = pet.page;
  move.flip = pet.flip;
  move.action = {pet.action.begin(), pet.action.end()};
  move.ani_index = pet.ani_index;
  ClientRequest::SendToHost(move);
}

uint32_t PetGameInstance::ParsePetAbility(wz::Node* info) {
  if (info == nullptr) {
    return 0;
  }
  auto flag = [info](const char16_t* key) -> bool {
    auto* node = info->get_child(key);
    return node != nullptr && node->type == wz::Type::Int &&
           static_cast<wz::Property<int32_t>*>(node)->get() != 0;
  };
  uint32_t flags = 0;
  if (flag(u"pickupMeso")) {
    flags |= kAbilityPickupMeso;
  }
  if (flag(u"pickupItem")) {
    flags |= kAbilityPickupItem;
  }
  if (flag(u"pickupOthers")) {
    flags |= kAbilityPickupOthers;
  }
  if (flag(u"longRange")) {
    flags |= kAbilityLongRange;
  }
  if (flag(u"sweepForDrop")) {
    flags |= kAbilitySweepForDrop;
  }
  if (flag(u"consumeHP")) {
    flags |= kAbilityConsumeHP;
  }
  if (flag(u"consumeMP")) {
    flags |= kAbilityConsumeMP;
  }
  if (flag(u"ignoreItems")) {
    flags |= kAbilityIgnoreItems;
  }
  return flags;
}

uint32_t PetGameInstance::LoadPetAbility(const std::u16string& pet_id) {
  const auto& self = CharacterGameInstance::self_;
  uint32_t flags = 0;
  if (self.pet_equip.has_value()) {
    flags = ParsePetAbility(EquipGameInstance::LoadEquipInfo(self.pet_equip->id));
  }
  if (flags == 0) {
    // 没有宠物装备能力时退回模板 info/pickupItem 的默认（捡道具）；金币默认允许。
    if (LoadPetCanPickItem(pet_id)) {
      flags |= kAbilityPickupItem;
    }
    flags |= kAbilityPickupMeso;
  }
  return flags;
}

bool PetGameInstance::UseAutoPotion(bool need_hp, bool need_mp) {
  if (!active_ || data_.empty()) {
    return false;
  }
  auto& items = PackageGameInstance::data_[static_cast<int>(ItemEnum::kConsume)];
  auto try_use = [&items](bool hp) -> bool {
    for (auto& itm : items) {
      if (itm->id.empty()) {
        continue;
      }
      auto* info = ItemGameInstance::LoadItemInfo(itm->id, 0);
      if (info == nullptr) {
        continue;
      }
      auto* spec = info->find(u"../spec");
      if (spec == nullptr) {
        continue;
      }
      const bool has = hp ? (spec->get_child(u"hp") != nullptr ||
                            spec->get_child(u"hpR") != nullptr)
                          : (spec->get_child(u"mp") != nullptr ||
                            spec->get_child(u"mpR") != nullptr);
      if (!has) {
        continue;
      }
      auto id = itm->id;
      if (ItemGameInstance::UseConsumeItem(id)) {
        ItemGameInstance::DecItemNum(itm, 1);
        return true;
      }
    }
    return false;
  };
  if (need_hp && try_use(true)) {
    return true;
  }
  if (need_mp && try_use(false)) {
    return true;
  }
  return false;
}


void PetGameInstance::HandleRemote(uint64_t client_id,
                                   const PetSaveData& state) {
  if (state.item_id == 0 || !state.active) {
    RemoveRemote(client_id);
    return;
  }
  auto id_text = std::format("{:08d}", state.item_id);
  GamePet pet;
  pet.id = {id_text.begin(), id_text.end()};
  pet.name = state.name;
  pet.level = state.level == 0 ? 1 : state.level;
  pet.fullness = state.fullness;
  pet.closeness = state.closeness;
  pet.index = -1;
  if (auto old = others_.find(client_id);
      old != others_.end() && old->second.id == pet.id) {
    // 同一只宠物：保留移动/动画状态，只更新状态值。
    pet = old->second;
    pet.name = state.name;
    pet.level = state.level == 0 ? 1 : state.level;
    pet.fullness = state.fullness;
    pet.closeness = state.closeness;
  } else {
    if (auto it = CharacterGameInstance::others_.find(client_id);
        it != CharacterGameInstance::others_.end()) {
      pet.pos = it->second.g_character.pos;
      pet.page = it->second.g_character.page;
      pet.flip = it->second.g_character.flip == 1;
    } else {
      pet.pos = CharacterGameInstance::self_.pos;
      pet.page = CharacterGameInstance::self_.page;
    }
    pet.fh = 0;
    pet.fall = true;
  }
  others_[client_id] = pet;
}

void PetGameInstance::RemoveRemote(uint64_t client_id) {
  others_.erase(client_id);
}

void PetGameInstance::SyncActiveState() {
  if (!active_ || pet_id_.empty()) {
    return;
  }
  auto id = static_cast<uint32_t>(
      std::stoi(std::string{pet_id_.begin(), pet_id_.end()}));
  auto& state = pets_[id];
  state.item_id = id;
  if (auto* pet = FindPet(0); pet != nullptr) {
    if (!pet->name.empty()) {
      state.name = pet->name;
    }
    state.level = pet->level == 0 ? 1 : pet->level;
    state.fullness = pet->fullness;
  }
}

void PetGameInstance::Load(const CharacterSaveData& cs) {
  pets_.clear();
  pet_id_.clear();
  active_ = false;
  data_.clear();
  for (const auto& state : cs.pets) {
    if (state.item_id == 0) {
      continue;
    }
    pets_[state.item_id] = state;
  }
  for (auto&& [id, state] : pets_) {
    if (!state.active) {
      continue;
    }
    auto id_text = std::format("{:08d}", id);
    pet_id_ = {id_text.begin(), id_text.end()};
    active_ = true;
    break;
  }
  Refresh();
}

void PetGameInstance::Save(CharacterSaveData& cs) {
  SyncActiveState();
  cs.pets.clear();
  uint32_t active_id = 0;
  if (active_ && !pet_id_.empty()) {
    active_id = static_cast<uint32_t>(
        std::stoi(std::string{pet_id_.begin(), pet_id_.end()}));
  }
  for (auto&& [id, state] : pets_) {
    auto saved = state;
    saved.item_id = id;
    saved.active = id == active_id;
    cs.pets.push_back(saved);
  }
}

void PetGameInstance::Toggle(const std::u16string& item_id) {
  if (!IsPetItem(item_id)) {
    return;
  }
  auto id = static_cast<uint32_t>(
      std::stoi(std::string{item_id.begin(), item_id.end()}));
  SyncActiveState();
  if (active_ && pet_id_ == item_id) {
    // 再点一次收回宠物（状态留在 pets_ 里）。
    if (auto it = pets_.find(id); it != pets_.end()) {
      it->second.active = false;
    }
    active_ = false;
    pet_id_.clear();
    data_.clear();
    SendToServer();
    return;
  }
  if (active_ && !pet_id_.empty()) {
    auto old = static_cast<uint32_t>(
        std::stoi(std::string{pet_id_.begin(), pet_id_.end()}));
    if (auto it = pets_.find(old); it != pets_.end()) {
      it->second.active = false;
    }
  }
  auto& state = pets_[id];
  state.item_id = id;
  if (state.name.empty()) {
    state.name = LoadPetName(item_id);
  }
  if (state.level == 0) {
    state.level = 1;
  }
  if (state.fullness == 0) {
    state.fullness = 100;
  }
  state.active = true;
  pet_id_ = item_id;
  active_ = true;
  Refresh();
  SendToServer();
}

PetSaveData* PetGameInstance::ActivePet() {
  if (!active_ || pet_id_.empty()) {
    return nullptr;
  }
  auto id = static_cast<uint32_t>(
      std::stoi(std::string{pet_id_.begin(), pet_id_.end()}));
  if (auto it = pets_.find(id); it != pets_.end()) {
    return &it->second;
  }
  return nullptr;
}

void PetGameInstance::SetFullness(uint8_t fullness) {
  if (auto* state = ActivePet(); state != nullptr) {
    state->fullness = fullness;
  }
}

bool PetGameInstance::Feed(const std::u16string& food_id) {
  if (!active_ || data_.empty() || food_id.size() < 8) {
    return false;
  }
  // 095：宠物食品的数据在 Item.wz/Consume/<xxxx>.img/<id>/spec，
  // 数字子节点列出能用这种食物的宠物 id，另外带 incRepleteness/incTameness。
  auto* image =
      WzResource::item_->find(u"Consume/" + food_id.substr(0, 4) + u".img");
  if (image == nullptr) {
    return false;
  }
  auto* node = image->get_child(food_id);
  if (node == nullptr) {
    return false;
  }
  auto* spec = node->get_child(u"spec");
  if (spec == nullptr || spec->get_children() == nullptr) {
    return false;
  }
  const auto pet_num = static_cast<uint32_t>(
      std::stoi(std::string{pet_id_.begin(), pet_id_.end()}));
  bool allowed = false;
  int repleteness = 0;
  int tameness = 0;
  for (auto&& [key, value] : *spec->get_children()) {
    if (key == u"incRepleteness") {
      if (value[0] != nullptr && value[0]->type == wz::Type::Int) {
        repleteness =
            static_cast<wz::Property<int32_t>*>(value[0])->get();
      }
      continue;
    }
    if (key == u"incTameness") {
      if (value[0] != nullptr && value[0]->type == wz::Type::Int) {
        tameness = static_cast<wz::Property<int32_t>*>(value[0])->get();
      }
      continue;
    }
    if (value[0] != nullptr && value[0]->type == wz::Type::Int &&
        static_cast<wz::Property<int32_t>*>(value[0])->get() ==
            static_cast<int32_t>(pet_num)) {
      allowed = true;
    }
  }
  if (!allowed || (repleteness == 0 && tameness == 0)) {
    return false;
  }
  auto* state = ActivePet();
  if (state == nullptr) {
    return false;
  }
  state->fullness = static_cast<uint8_t>(
      std::min<uint32_t>(100, static_cast<uint32_t>(state->fullness) +
                                  static_cast<uint32_t>(repleteness)));
  state->closeness = static_cast<uint8_t>(
      std::min<uint32_t>(100, static_cast<uint32_t>(state->closeness) +
                                  static_cast<uint32_t>(tameness)));
  // 吃食物的小动作（宠物图片里的 food/0..3）。
  if (auto* pet = FindPet(0); pet != nullptr) {
    pet->fullness = state->fullness;
    pet->closeness = state->closeness;
    pet->action = u"food";
    pet->ani_index = 0;
    pet->ani_time = 0;
    pet->action_end = Window::dt_now_ + 2000;
    pet->next_idle = Window::dt_now_ + 4000;
  }
  SendActionToServer(u"food", u"");
  SendToServer();
  return true;
}

void PetGameInstance::Refresh() {
  std::vector<GamePet> pets;
  if (!active_ || pet_id_.empty() || LoadPetNode(pet_id_) == nullptr) {
    if (!data_.empty()) {
      data_.clear();
    }
    return;
  }
  // 宠物道具必须还在现金栏里，否则视为失效。
  auto& cash = PackageGameInstance::data_[static_cast<int>(ItemEnum::kCash)];
  const bool owned = std::ranges::any_of(cash, [](const auto& item) {
    return !item->id.empty() && LoadPetNode(item->id) != nullptr &&
           item->id == pet_id_;
  });
  if (!owned) {
    // 道具被丢掉/卖掉：宠物消失，同时通知服务器让其他玩家也看不到。
    if (active_) {
      SendToServer();
    }
    active_ = false;
    if (!data_.empty()) {
      data_.clear();
    }
    return;
  }

  const auto* state = ActivePet();
  static const PetSaveData kEmptyState{};
  if (state == nullptr) {
    state = &kEmptyState;
  }
  GamePet pet;
  pet.id = pet_id_;
  pet.name = state->name.empty() ? LoadPetName(pet_id_) : state->name;
  pet.level = state->level == 0 ? 1 : state->level;
  pet.fullness = state->fullness;
  pet.closeness = state->closeness;
  // 已经在场的宠物保留移动/动画状态，避免刷新时瞬移。
  if (auto it = std::ranges::find_if(
          data_, [&pet](const GamePet& p) { return p.id == pet.id; });
      it != data_.end()) {
    pet = *it;
    // 名字/等级/饱食度以存档状态为准（喂食、改名等会更新它）。
    pet.name = state->name.empty() ? LoadPetName(pet_id_) : state->name;
    pet.level = state->level == 0 ? 1 : state->level;
    pet.fullness = state->fullness;
    pet.closeness = state->closeness;
    pet.index = 0;
  } else {
    auto& self = CharacterGameInstance::self_;
    pet.pos = self.pos;
    pet.fh = CharacterLogicSystem::self_fh_;
    pet.page = self.page;
    pet.flip = self.flip == 1;
  }
  pets.push_back(std::move(pet));

  // 列表没变化时不重建，避免外部持有的 GamePet 引用失效。
  if (pets.size() == data_.size() &&
      std::ranges::equal(pets, data_, [](const GamePet& a, const GamePet& b) {
        return a.id == b.id;
      })) {
    // 仍然把状态同步回 pets_（饱食度会随时间下降）。
    if (auto* p = FindPet(0); p != nullptr) {
      SetFullness(p->fullness);
    }
    return;
  }
  data_ = std::move(pets);
}

namespace {

std::u16string TrimCommand(const std::u16string& s) {
  auto begin = s.begin();
  auto end = s.end();
  while (begin != end && (*begin == u' ' || *begin == u'\t')) {
    ++begin;
  }
  while (begin != end && (*(end - 1) == u' ' || *(end - 1) == u'\t')) {
    --end;
  }
  return {begin, end};
}

std::u16string LowerCommand(std::u16string s) {
  for (auto& c : s) {
    if (c >= u'A' && c <= u'Z') {
      c = static_cast<char16_t>(c - u'A' + u'a');
    }
  }
  return s;
}

bool IsAllDigits(const std::u16string& s) {
  return !s.empty() && std::ranges::all_of(s, [](char16_t c) {
    return c >= u'0' && c <= u'9';
  });
}

int32_t ReadNodeInt(wz::Node* node) {
  if (node == nullptr || node->type != wz::Type::Int) {
    return 0;
  }
  return static_cast<wz::Property<int32_t>*>(node)->get();
}

std::u16string ReadNodeString(wz::Node* node) {
  if (node == nullptr || node->type != wz::Type::String) {
    return u"";
  }
  return static_cast<wz::Property<std::u16string>*>(node)->get();
}

// "sit" / "talk|chat|say" -> 拆成别名列表
std::vector<std::u16string> SplitAliases(const std::u16string& text) {
  std::vector<std::u16string> r;
  size_t start = 0;
  while (true) {
    auto pos = text.find(u'|', start);
    r.push_back(text.substr(start, pos == std::u16string::npos
                                      ? std::u16string::npos
                                      : pos - start));
    if (pos == std::u16string::npos) {
      break;
    }
    start = pos + 1;
  }
  return r;
}

}  // namespace

int32_t PetGameInstance::LoadPetChatBalloonIndex(const std::u16string& id) {
  auto* node = LoadPetNode(id);
  if (node == nullptr) {
    return 0;
  }
  auto* info = node->get_child(u"info");
  if (info == nullptr) {
    return 0;
  }
  return ReadNodeInt(info->get_child(u"chatBalloon"));
}

bool PetGameInstance::RunCommand(const std::u16string& raw) {
  if (!active_ || data_.empty() || pet_id_.empty()) {
    return false;
  }
  auto* pet = FindPet(0);
  if (pet == nullptr) {
    return false;
  }
  // 095 ParseCommand：正在播一次性动作时不响应命令。
  if (pet->action_end != 0) {
    return false;
  }
  auto* node = LoadPetNode(pet_id_);
  auto* interact = node == nullptr ? nullptr : node->get_child(u"interact");
  if (interact == nullptr || interact->get_children() == nullptr) {
    return false;
  }
  const auto pet_key = LoadPetImageKey(pet_id_);
  if (pet_key.empty()) {
    return false;
  }
  auto text = TrimCommand(raw);
  if (text.empty()) {
    return false;
  }
  auto ltext = LowerCommand(text);
  // 095：自定义了名字的宠物可以先喊名字（"moco sit"），再允许 ! , ~ 前缀。
  const auto lname = LowerCommand(pet->name);
  if (!lname.empty() && ltext.size() > lname.size() &&
      ltext.starts_with(lname)) {
    text = TrimCommand(text.substr(lname.size()));
    ltext = LowerCommand(text);
  }
  while (!ltext.empty() &&
         (ltext.front() == u'!' || ltext.front() == u',' ||
          ltext.front() == u'~')) {
    text = TrimCommand(text.substr(1));
    ltext = LowerCommand(text);
  }
  if (ltext.empty()) {
    return false;
  }

  for (auto&& [key, list] : *interact->get_children()) {
    if (!IsAllDigits(key) || list.empty() || list[0] == nullptr) {
      continue;
    }
    auto* entry = list[0];
    // 等级区间外的条目跳过（095：GetLevel 不在 [l0,l1] 内不算命中）。
    if (pet->level < ReadNodeInt(entry->get_child(u"l0")) ||
        pet->level > ReadNodeInt(entry->get_child(u"l1"))) {
      continue;
    }
    auto command = ReadNodeString(entry->get_child(u"command"));
    if (command.empty()) {
      continue;
    }
    auto* aliases_node = WzResource::string_->find(
        u"PetCommand.img/" + pet_key + u"/" + command);
    if (aliases_node == nullptr) {
      continue;
    }
    bool matched = false;
    for (const auto& alias : SplitAliases(ReadNodeString(aliases_node))) {
      if (LowerCommand(TrimCommand(alias)) == ltext) {
        matched = true;
        break;
      }
    }
    if (!matched) {
      continue;
    }

    // 095：prob 是成功概率，成功/失败播不同的动作和台词。
    const int32_t prob = ReadNodeInt(entry->get_child(u"prob"));
    const int32_t inc = ReadNodeInt(entry->get_child(u"inc"));
    std::uniform_int_distribution<int> roll(0, 99);
    const bool success = roll(RandomGameInstance::gen_) < prob;
    auto* branch = entry->get_child(success ? u"success" : u"fail");
    auto* lines = branch == nullptr ? nullptr : branch->get_child(u"0");
    if (lines != nullptr && lines->get_children() != nullptr) {
      std::u16string act;
      std::vector<std::u16string> dialog_keys;
      for (auto&& [lk, lv] : *lines->get_children()) {
        if (lv.empty() || lv[0] == nullptr) {
          continue;
        }
        if (lk == u"act") {
          act = ReadNodeString(lv[0]);
        } else if (IsAllDigits(lk)) {
          // 子节点是台词键（如 c1_f1），文本在 String.wz/PetDialog.img 下。
          auto key_text = ReadNodeString(lv[0]);
          if (!key_text.empty()) {
            dialog_keys.push_back(std::move(key_text));
          }
        }
      }
      // 随机挑一句 PetDialog 台词。
      std::u16string dialog;
      if (!dialog_keys.empty()) {
        std::uniform_int_distribution<size_t> pick(0, dialog_keys.size() - 1);
        auto* dialog_node = WzResource::string_->find(
            u"PetDialog.img/" + pet_key + u"/" +
            dialog_keys[pick(RandomGameInstance::gen_)]);
        dialog = ReadNodeString(dialog_node);
      }
      if (!act.empty() && LoadPetNode(pet_id_)->get_child(act) != nullptr) {
        pet->action = act;
        pet->ani_index = 0;
        pet->ani_time = 0;
        pet->action_end = Window::dt_now_ + 1500;
        pet->next_idle = Window::dt_now_ + 4000;
      }
      if (!dialog.empty()) {
        auto idx_text = std::to_string(LoadPetChatBalloonIndex(pet_id_));
        GameChatballoon balloon;
        balloon.text = dialog;
        balloon.path = std::u16string{idx_text.begin(), idx_text.end()};
        balloon.w = 120.0f;
        balloon.size = 14;
        balloon.destroy = Window::dt_now_ + 5000;
        pet->chatballoon = balloon;
      }
      if (!act.empty() || !dialog.empty()) {
        SendActionToServer(act, dialog);
      }
      // 成功时按 inc 增加亲密度。
      if (success && inc > 0) {
        if (auto* state = ActivePet(); state != nullptr) {
          state->closeness = static_cast<uint8_t>(
              std::min<uint32_t>(100, static_cast<uint32_t>(state->closeness) +
                                          static_cast<uint32_t>(inc)));
          if (auto* self_pet = FindPet(0); self_pet != nullptr) {
            self_pet->closeness = state->closeness;
          }
        }
      }
    }
    return true;
  }
  return false;
}

}  // namespace sdlms

