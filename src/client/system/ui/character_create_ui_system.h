#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_character.h"
#include "text_input_ui_system.h"
#include <string>
#include <vector>
class CharacterCreateUiSystem {
private:
  static std::vector<std::u16string> LoadDefaultFace();
  static std::vector<std::u16string> LoadDefaultHair();
  static std::vector<std::u16string> LoadDefaultTop();
  static std::vector<std::u16string> LoadDefaultBottom();
  static std::vector<std::u16string> LoadDefaultShoes();
  static std::vector<std::u16string> LoadDefaultWeapon();

  static std::vector<std::u16string> LoadHaircolors();
  static bool BackAnimate();

  static SDL_FPoint LoadPos();
  static void RenderBackgrnd();
  static void RenderButton();
  static void RenderEffect();
  static void RenderBanner();
  static void RenderStat();
  static void RenderCharacter();
  static void RenderCustom();
  static void RenderName();

  enum class ChooseType {
    kGender,
    kFace,
    kHairstyle,
    kHaircolor,
    kSkin,
    kTop,
    kBottom,
    kShoes,
    kWeapon,
  };

  static void RenderCustomItem(float cx, float cy, ChooseType type);

  static void EventButtonFacePrev();
  static void EventButtonFaceNext();

  static void EventButtonHairPrev();
  static void EventButtonHairNext();

  static void EventButtonHairColorPrev();
  static void EventButtonHairColorNext();

  static void EventButtonSkinColorPrev();
  static void EventButtonSkinColorNext();

  static void EventButtonTopPrev();
  static void EventButtonTopNext();

  static void EventButtonBottomPrev();
  static void EventButtonBottomNext();

  static void EventButtonShoesPrev();
  static void EventButtonShoesNext();

  static void EventButtonWeaponPrev();
  static void EventButtonWeaponNext();

  static void EventButtonStrInc();
  static void EventButtonStrDec();

  static void EventButtonDexInc();
  static void EventButtonDexDec();

  static void EventButtonIntInc();
  static void EventButtonIntDec();

  static void EventButtonLukInc();
  static void EventButtonLukDec();

  static void EventButtonOk();
  static void EventButtonCancel();
  static void EventButtonBack();

  static bool EventButton(SDL_Event *Event);
  static bool EventButtonCustom(SDL_Event *Event);

public:
  static inline TextInput name;

  static inline int str_point;
  static inline int dex_point;
  static inline int int_point;
  static inline int luk_point;
  static inline int remain_point;

  static inline int gender;
  static inline int choose_index;

  static void ResetCharacter(bool g);
  static bool Render();
  static bool Event(SDL_Event *Event);
  static bool Run();

  static inline GameCharacter g_character;
};