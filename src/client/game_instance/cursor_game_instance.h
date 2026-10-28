#pragma once

#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <optional>
#include <string>

using namespace fbs;

class CursorGameInstance {
public:
  static inline std::u16string cursor_type_ = u"0";
  static inline uint8_t cursor_index_;
  static inline uint32_t cursor_time_;

  static inline bool (*cursor_ui)(void);
  static inline bool cursor_vscr_;
  static inline bool (*modal_overlay)(void);

  // 点击装备,背包,技能,键盘按钮
  enum class CursorHandType {
    kEquipment,
    kDeco,
    kPackage,
    kSkill,
    kKeybind,
  };
  enum class CursorKeybindType { // sub_val
    kKeybindUiMenu,
    kKeybindFaceHit,
    kKeybindFaceSmile,
    kKeybindFaceTroubled,
    kKeybindFaceCry,
    kKeybindFaceAngry,
    kKeybindFaceBewildered,
    kKeybindFaceStunned,
    kKeybindUiToAll,
    kKeybindUiToParty,
    kKeybindUiQuest,
    kKeybindUiWorldMap,
    kKeybindUiEquip,
    kKeybindUiPackage,
    kKeybindUiBuddy,
    kKeybindUiParty,
    kKeybindUiMapleChat,
    kKeybindUiQuickSlot,
    kKeybindUiKeyBind,
    kKeybindUiCharacterStat,
    kKeybindUiWhisper,
    kKeybindUiSkill,
    kKeybindUiCraft,
    kKeybindUiMailBox,
    kKeybindUiChatPlus,
    kKeybindUiMiniMap,
    kKeybindUiChangeChannel,
    kKeybindUiOk,
    kKeybindActionPick,
    kKeybindActionSit,
    kKeybindActionAttack,
    kKeybindActionJump,
    kKeybindActionHarvest,
    kKeybindActionUp,
    kKeybindActionDown,
    kKeybindActionLeft,
    kKeybindActionRight,
    kKeybindFaceVomit,
  };
  struct CursorHandData {
    CursorHandType type;
    uint32_t val;
    uint32_t sub_val;
  };
  enum class CursorHandNetType {
    kDrop,
  };
  struct CursorHandNetData {
    CursorHandNetType type;
    uint64_t id;
  };

  static inline std::optional<CursorHandNetData> cursor_hand_net_;
  static inline std::optional<CursorHandData> cursor_hand_;
};