#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

// The in-game options window, described by `UI/Options.img`.
//
// The wz carries the whole layout: every tab (`graphic`, `sound`, `game`,
// `social`, `controller`) owns child nodes named `<kind>:<name>` that hold
// their own geometry (`lt`/`rb`, or `width` for sliders) and, for labels, the
// `SID` of the text in `String/StringTable.img`. The window therefore parses
// the data instead of hard-coding positions, keeps every value addressed by
// "<tab>/<element>" so it can be saved, and pushes the values other systems
// care about (audio gains, chat font size, enemy info) on Apply().
class OptionUiSystem : public UiComponentHost<OptionUiSystem> {
 public:
  enum class TabEnum : uint8_t {
    kGraphic,
    kSound,
    kGame,
    kSocial,
    kController,
  };

  // wz 里 5 个 tab 的节点名，顺序与 TabEnum 一致
  static std::u16string LoadTabName(uint8_t tab);
  static wz::Node* LoadTabNode(uint8_t tab);

  // ---- 设置值（key = "<tab>/<element>"）----
  static int32_t Value(uint8_t tab, const std::u16string& name);
  static void SetValue(uint8_t tab, const std::u16string& name, int32_t val);

  // 从存档读入 / 写回存档，并把影响其它系统的项推下去
  static void Load();
  static void Save();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);
  static bool CursorIn();
  static void Toggle();
  static UIComponents& GetComponent();

  // ---- 其它系统查询的选项 ----
  static uint8_t HpWarnPercent();
  static uint8_t MpWarnPercent();
  // 0 = No Labels, 1 = Name Only, 2 = HP Only, 3 = Name and HP
  static uint8_t EnemyInfo();
  static uint8_t ChatFontSize();

 private:
  // 一条 wz 里的控件描述
  struct Element {
    std::u16string kind;  // check / slider / combo / staticText / edit / button
    std::u16string name;
    wz::Node* node = nullptr;
    SDL_FRect rect = {};  // 相对面板左上角
    int32_t width = 0;    // slider 轨道长度
    int32_t f_type = 0;   // staticText 字体类型
    int32_t align_h = 0;  // 0 左 / 1 中 / 2 右
    int32_t align_v = 1;  // 0 上 / 1 中 / 2 下
    std::u16string text;  // staticText 的文案（SID 查表或 text 直接给）
    std::vector<std::u16string> selects;  // combo 选项
  };
  static std::vector<Element> LoadElements(uint8_t tab);
  static std::u16string LoadText(const std::u16string& sid);
  static int32_t DefaultValue(const Element& element);
  static std::u16string MakeKey(uint8_t tab, const std::u16string& name);

  static void RenderBackgrnd();
  static void RenderContent();
  static void RenderComboList();

  static bool EventUI(SDL_Event* event);
  static void EventButtonOk();
  static void EventButtonCancel();
  static void EventButtonClose();
  static void EventTop();
  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);
  static void EventMotionBtAudio(SDL_Event* event);
  static void EventSlider(SDL_Event* event);
  static std::optional<uint8_t> ClickCombo(const Element& element,
                                           SDL_FPoint mouse);
  static SDL_FPoint LoadWh();

  // 值 / 存档
  static std::flat_map<std::u16string, int32_t>& Values();
  static void Apply();
  static inline bool loaded_ = false;

  static inline std::optional<SDL_FPoint> drag_;
  static inline bool dragging_slider_ = false;
  static inline std::optional<std::u16string> open_combo_;  // 展开的 combo 名
  static inline uint8_t active_tab_ = 0;
  static inline SDL_FPoint pos_ = {0, 0};
};

}  // namespace sdlms
