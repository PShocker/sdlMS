#include "src/client/game_instance/keyboard_game_instance.h"

#include "src/client/game/game_input.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"

#include <SDL3/SDL_scancode.h>

#include <format>
#include <optional>
#include <string>

namespace sdlms {

void KeyboardGameInstance::Reset() {
  data_.clear();
  InsertInput(SDL_SCANCODE_ESCAPE, {.type = "ui", .val = "menu"});
  InsertInput(SDL_SCANCODE_F1, {.type = "face", .val = "hit"});
  InsertInput(SDL_SCANCODE_F2, {.type = "face", .val = "smile"});
  InsertInput(SDL_SCANCODE_F3, {.type = "face", .val = "troubled"});
  InsertInput(SDL_SCANCODE_F4, {.type = "face", .val = "cry"});
  InsertInput(SDL_SCANCODE_F5, {.type = "face", .val = "angry"});
  InsertInput(SDL_SCANCODE_F6, {.type = "face", .val = "bewildered"});
  InsertInput(SDL_SCANCODE_F7, {.type = "face", .val = "stunned"});
  InsertInput(SDL_SCANCODE_1, {.type = "ui", .val = "toAll"});
  InsertInput(SDL_SCANCODE_2, {.type = "ui", .val = "toParty"});
  InsertInput(SDL_SCANCODE_3, {.type = "ui", .val = "toBuddy"});
  InsertInput(SDL_SCANCODE_Q, {.type = "ui", .val = "quest"});
  InsertInput(SDL_SCANCODE_W, {.type = "ui", .val = "worldMap"});
  InsertInput(SDL_SCANCODE_E, {.type = "ui", .val = "equip"});
  InsertInput(SDL_SCANCODE_I, {.type = "ui", .val = "package"});
  InsertInput(SDL_SCANCODE_O, {.type = "ui", .val = "buddy"});
  InsertInput(SDL_SCANCODE_P, {.type = "ui", .val = "party"});
  InsertInput(SDL_SCANCODE_LEFTBRACKET, {.type = "ui", .val = "mapleChat"});
  InsertInput(SDL_SCANCODE_RIGHTBRACKET, {.type = "ui", .val = "quickSlot"});
  InsertInput(SDL_SCANCODE_BACKSLASH, {.type = "ui", .val = "keyBind"});
  InsertInput(SDL_SCANCODE_S, {.type = "ui", .val = "characterStat"});
  InsertInput(SDL_SCANCODE_H, {.type = "ui", .val = "whisper"});
  InsertInput(SDL_SCANCODE_K, {.type = "ui", .val = "skill"});
  InsertInput(SDL_SCANCODE_L, {.type = "ui", .val = "craft"});
  InsertInput(SDL_SCANCODE_SEMICOLON, {.type = "ui", .val = "mailBox"});
  InsertInput(SDL_SCANCODE_APOSTROPHE, {.type = "ui", .val = "chat+"});
  InsertInput(SDL_SCANCODE_Z, {.type = "action", .val = "pick"});
  InsertInput(SDL_SCANCODE_X, {.type = "action", .val = "sit"});
  InsertInput(SDL_SCANCODE_M, {.type = "ui", .val = "miniMap"});
  InsertInput(SDL_SCANCODE_COMMA, {.type = "ui", .val = "changeChannel"});
  InsertInput(SDL_SCANCODE_LCTRL, {.type = "action", .val = "attack"});
  InsertInput(SDL_SCANCODE_LALT, {.type = "action", .val = "jump"});
  InsertInput(SDL_SCANCODE_SPACE, {.type = "action", .val = "harvest"});
  InsertInput(SDL_SCANCODE_UP, {.type = "action", .val = "up"});
  InsertInput(SDL_SCANCODE_DOWN, {.type = "action", .val = "down"});
  InsertInput(SDL_SCANCODE_LEFT, {.type = "action", .val = "left"});
  InsertInput(SDL_SCANCODE_RIGHT, {.type = "action", .val = "right"});
}

void KeyboardGameInstance::Load() {
  Reset();
}

void KeyboardGameInstance::Load(const CharacterSaveData& cs) {
  data_ = {};
  for (auto& k : cs.keys) {
    data_[(SDL_Scancode)k.scan_code] = {
        .type = k.type,
        .val = k.val,
        .sub_val = k.sub_val,
    };
  }
}

SDL_Scancode KeyboardGameInstance::FindKeyByVal(const std::string& target_val) {
  for (const auto& [scancode, binding] : data_) {
    if (binding.val == target_val) {
      return scancode;
    }
  }
  return SDL_SCANCODE_UNKNOWN;  // 未找到
}

GameInput KeyboardGameInstance::FindInputByCursor(int i) {
  GameInput input;
  switch ((CursorGameInstance::CursorKeybindType)i) {
    case CursorGameInstance::CursorKeybindType::kKeybindActionAttack: {
      input = {
          .type = "action",
          .val = "attack",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionSit: {
      input = {
          .type = "action",
          .val = "sit",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionJump: {
      input = {
          .type = "action",
          .val = "jump",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionPick: {
      input = {
          .type = "action",
          .val = "pick",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionUp: {
      input = {
          .type = "action",
          .val = "up",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionDown: {
      input = {
          .type = "action",
          .val = "down",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionLeft: {
      input = {
          .type = "action",
          .val = "left",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionRight: {
      input = {
          .type = "action",
          .val = "right",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiWorldMap: {
      input = {
          .type = "ui",
          .val = "worldMap",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiPackage: {
      input = {
          .type = "ui",
          .val = "package",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiCharacterStat: {
      input = {
          .type = "ui",
          .val = "characterStat",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiKeyBind: {
      input = {
          .type = "ui",
          .val = "keyBind",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiSkill: {
      input = {
          .type = "ui",
          .val = "skill",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiEquip: {
      input = {
          .type = "ui",
          .val = "equip",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiCraft: {
      input = {
          .type = "ui",
          .val = "craft",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiQuest: {
      input = {
          .type = "ui",
          .val = "quest",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiOk: {
      input = {
          .type = "ui",
          .val = "ok",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiBuddy: {
      input = {
          .type = "ui",
          .val = "buddy",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceVomit: {
      input = {
          .type = "face",
          .val = "vomit",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiMenu: {
      input = {
          .type = "ui",
          .val = "menu",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceHit: {
      input = {
          .type = "face",
          .val = "hit",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceSmile: {
      input = {
          .type = "face",
          .val = "smile",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceTroubled: {
      input = {
          .type = "face",
          .val = "troubled",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceCry: {
      input = {
          .type = "face",
          .val = "cry",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceAngry: {
      input = {
          .type = "face",
          .val = "angry",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceBewildered: {
      input = {
          .type = "face",
          .val = "bewildered",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindFaceStunned: {
      input = {
          .type = "face",
          .val = "stunned",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiToAll: {
      input = {
          .type = "ui",
          .val = "toAll",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiToParty: {
      input = {
          .type = "ui",
          .val = "toParty",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiParty: {
      input = {
          .type = "ui",
          .val = "party",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiMapleChat: {
      input = {
          .type = "ui",
          .val = "mapleChat",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiQuickSlot: {
      input = {
          .type = "ui",
          .val = "quickSlot",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiWhisper: {
      input = {
          .type = "ui",
          .val = "whisper",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiMailBox: {
      input = {
          .type = "ui",
          .val = "mailBox",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiChatPlus: {
      input = {
          .type = "ui",
          .val = "chat+",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiMiniMap: {
      input = {
          .type = "ui",
          .val = "miniMap",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindUiChangeChannel: {
      input = {
          .type = "ui",
          .val = "changeChannel",
      };
      break;
    }
    case CursorGameInstance::CursorKeybindType::kKeybindActionHarvest: {
      input = {
          .type = "action",
          .val = "harvest",
      };
      break;
    }
  }
  return input;
}

void KeyboardGameInstance::InsertInput(SDL_Scancode c, GameInput i) {
  EraseInput(c);
  EraseInput(i);
  switch (c) {
    case SDL_SCANCODE_LCTRL:
    case SDL_SCANCODE_RCTRL:
      data_[SDL_SCANCODE_LCTRL] = i;
      data_[SDL_SCANCODE_RCTRL] = i;
      break;
    case SDL_SCANCODE_LALT:
    case SDL_SCANCODE_RALT:
      data_[SDL_SCANCODE_LALT] = i;
      data_[SDL_SCANCODE_RALT] = i;
      break;
    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT:
      data_[SDL_SCANCODE_LSHIFT] = i;
      data_[SDL_SCANCODE_RSHIFT] = i;
      break;
    default:
      data_[c] = i;
      break;
  }
}

void KeyboardGameInstance::EraseInput(SDL_Scancode c) {
  switch (c) {
    case SDL_SCANCODE_LCTRL:
    case SDL_SCANCODE_RCTRL:
      data_.erase(SDL_SCANCODE_LCTRL);
      data_.erase(SDL_SCANCODE_RCTRL);
      break;
    case SDL_SCANCODE_LALT:
    case SDL_SCANCODE_RALT:
      data_.erase(SDL_SCANCODE_LALT);
      data_.erase(SDL_SCANCODE_RALT);
      break;
    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT:
      data_.erase(SDL_SCANCODE_LSHIFT);
      data_.erase(SDL_SCANCODE_RSHIFT);
      break;
    default:
      data_.erase(c);
      break;
  }
}

void KeyboardGameInstance::EraseInput(GameInput i) {
  std::erase_if(data_, [&](const auto& kv) {
    return kv.second == i;  // 举例字段
  });
}

std::optional<GameInput> KeyboardGameInstance::LoadCursorInput() {
  if (!CursorGameInstance::cursor_hand_.has_value()) {
    return std::nullopt;
  }
  auto& cursor_hand = CursorGameInstance::cursor_hand_.value();
  std::string id;
  switch (cursor_hand.type) {
    case CursorGameInstance::CursorHandType::kKeybind: {
      if (KeyboardGameInstance::data_.contains((SDL_Scancode)cursor_hand.val)) {
        return KeyboardGameInstance::data_.at((SDL_Scancode)cursor_hand.val);
      } else {
        return FindInputByCursor(cursor_hand.sub_val);
      }
      break;
    }
    case CursorGameInstance::CursorHandType::kPackage: {
      auto& item =
          PackageGameInstance::data_[cursor_hand.val][cursor_hand.sub_val];
      id = std::string{item->id.begin(), item->id.end()};
      if (id.starts_with("02")) {
        if (id.starts_with("0204") || id.starts_with("0206") ||
            id.starts_with("0207")) {
          break;
        }
        GameInput input{
            .type = "item",
            .val = id,
        };
        return input;
      }
      break;
    }
    case CursorGameInstance::CursorHandType::kSkill: {
      id = std::format("{:07d}", cursor_hand.sub_val);
      GameInput input{
          .type = "skill",
          .val = id,
      };
      return input;
      break;
    }
    default: {
      break;
    }
  }
  return std::nullopt;
}

}  // namespace sdlms
