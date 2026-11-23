#pragma once

#include "src/client/game/game_pet.h"
#include "src/client/game/game_save.h"

#include <wz/Node.h>

#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

namespace sdlms {

// 宠物数据与运行时列表。
//
// 095 里 CPet 由 CPetTemplate + 角色身上的宠物状态创建：宠物道具放在现金栏，
// 使用后激活（双击道具），名字/等级/饱食度存在角色数据里。这里对应地保存
// pets_（每只宠物各自的状态）+ pet_id_/active_（当前在场的那只），进图时重建。
class PetGameInstance {
 public:
  static inline std::vector<GamePet> data_;

  // 拥有的宠物状态，key = 宠物道具 id 的数值（如 5000000，对应 05000000）；
  // 没激活的宠物也保留名字/等级/饱食度，切换宠物不会丢。
  static inline std::flat_map<uint32_t, PetSaveData> pets_;
  // 当前激活的宠物道具 id（8 位字符串），空 = 没有宠物在场。
  static inline std::u16string pet_id_;
  static inline bool active_ = false;

  // 其他玩家的宠物（服务器同步）：key = client_id。
  static inline std::flat_map<uint64_t, GamePet> others_;

  // 把自己的宠物状态上报服务器（进图/切换/喂食后），供其他玩家看到。
  static void SendToServer();
  // 095 CPet::DoAction 的 bSend 包：命令/喂食时把动作和气泡台词广播给同地图其他人。
  static void SendActionToServer(const std::u16string& action,
                                 const std::u16string& chat);
  // 095 CPet 的移动包：把自己的宠物位置/当前帧同步给其他玩家。
  static void SendMoveToServer();

  // 095 CItemInfo::dwPetAbilityFlag：宠物装备 info 里的能力位。
  enum PetAbilityFlag : uint32_t {
    kAbilityPickupMeso = 1u << 0,    // info/pickupMeso
    kAbilityPickupItem = 1u << 1,    // info/pickupItem
    kAbilityPickupOthers = 1u << 2,  // info/pickupOthers
    kAbilityLongRange = 1u << 3,     // info/longRange
    kAbilitySweepForDrop = 1u << 4,  // info/sweepForDrop
    kAbilityConsumeHP = 1u << 5,     // info/consumeHP
    kAbilityConsumeMP = 1u << 6,     // info/consumeMP
    kAbilityIgnoreItems = 1u << 7,   // info/ignoreItems
  };
  // 解析宠物装备的 info 节点（095 RegisterEquipItemInfo 的读法）。
  static uint32_t ParsePetAbility(wz::Node* info);
  // 当前宠物实际生效的能力：宠物装备的能力位；没有装备时退回模板 info/pickupItem 的默认。
  static uint32_t LoadPetAbility(const std::u16string& pet_id);
  // 095 consumeHP/consumeMP：主人 HP/MP 低时从背包自动用一瓶药。
  static bool UseAutoPotion(bool need_hp, bool need_mp);
  // 收到 ServerCharacterPet（或其他玩家离场）。
  static void HandleRemote(uint64_t client_id, const PetSaveData& state);
  static void RemoveRemote(uint64_t client_id);

  // 从存档恢复宠物状态并按需生成宠物。
  static void Load(const CharacterSaveData& cs);
  // 把当前宠物状态写回存档结构。
  static void Save(CharacterSaveData& cs);

  // 使用宠物道具：切换该宠物的激活状态（095：宠物道具激活后才会跟着主人）。
  static void Toggle(const std::u16string& item_id);

  // 095 CPet::OnValidateStat：更新当前宠物的饱食度。
  static void SetFullness(uint8_t fullness);

  // 喂宠物食品（095：Item.wz/Consume/0212.img/02120000/spec 里列出可喂的宠物 id
  // + incRepleteness/incTameness）。成功返回 true，调用方负责扣道具。
  static bool Feed(const std::u16string& food_id);

  // 095 CPet::ParseCommand：聊天框里输入宠物命令（String.wz/PetCommand.img 的别名，
  // 如 sit/talk/iloveyou），按 Item.wz/Pet/<id>.img/interact 里的 l0/l1/prob/success/fail
  // 播放一次性动作并用 PetDialog 的台词冒泡。命中命令返回 true。
  static bool RunCommand(const std::u16string& text);

  // 095 CPetTemplate::nChatBalloon：模板 info/chatBalloon（ChatBalloon.img 的样式号）。
  static int32_t LoadPetChatBalloonIndex(const std::u16string& id);

  // 按当前激活状态重建宠物列表；在场宠物保留位置/动作状态。
  static void Refresh();
  static void Clear();

  // 8 位道具 id 是否是宠物道具（Item.wz/Pet/<7 位 id>.img）。
  static bool IsPetItem(const std::u16string& id);
  static wz::Node* LoadPetNode(const std::u16string& id);
  static std::u16string LoadPetName(const std::u16string& id);
  // 8 位道具 id -> Item.wz/Pet 里的 7 位图片键（05000000 -> 5000000）；非宠物返回空。
  static std::u16string LoadPetImageKey(const std::u16string& id);
  // 095 CPetTemplate::nNameTag：模板 info/nameTag，-1 表示没有（用普通黑底名签）。
  static int32_t LoadPetNameTagIndex(const std::u16string& id);
  // 095 CPetTemplate::bPickUpItem：模板 info/pickupItem，宠物会不会帮捡东西。
  static bool LoadPetCanPickItem(const std::u16string& id);

  static GamePet* FindPet(int32_t index);
  // 当前激活宠物的存档状态；没有激活宠物时返回 nullptr。
  static PetSaveData* ActivePet();

 private:
  // 把在场宠物的名字/等级/饱食度同步回 pets_。
  static void SyncActiveState();
};

}  // namespace sdlms
