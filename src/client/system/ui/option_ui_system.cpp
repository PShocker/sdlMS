#include "src/client/system/ui/option_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/chat_log_ui_system.h"
#include "src/client/system/ui/component/button.h"
#include "src/client/system/ui/component/check_box.h"
#include "src/client/system/ui/component/combo_box.h"
#include "src/client/system/ui/component/slider.h"
#include "src/client/system/ui/component/tab.h"
#include "src/client/system_instance/game_save_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <flat_map>
#include <string>
#include <utility>
#include <vector>

namespace sdlms {

namespace {

// 面板尺寸（Options.img/backgrnd 的 canvas 大小）
constexpr float kPanelW = 375.0f;
constexpr float kPanelH = 495.0f;
// tab 个数
constexpr uint8_t kTabCount = 5;
// 字号：标签 fType 280、说明/数值 fType 283
constexpr uint8_t kLabelSize = 12;
constexpr uint8_t kSmallSize = 11;
// 行距
constexpr float kLineSpacing = 1.2f;
// 下拉列表每项高度，也用作滑块轨道的点击高度
constexpr float kComboItemH = 14.0f;
// 滑块的取值范围
constexpr int32_t kSliderMax = 100;

// 元素的矩形：多数用 lt/rb，部分说明文字用 pos/width/height
SDL_FRect LoadRect(wz::Node* node) {
  if (node->get_child(u"lt") != nullptr) {
    auto lt =
        static_cast<wz::Property<wz::WzVec2D>*>(node->get_child(u"lt"))->get();
    auto rb =
        static_cast<wz::Property<wz::WzVec2D>*>(node->get_child(u"rb"))->get();
    return SDL_FRect{static_cast<float>(lt.x), static_cast<float>(lt.y),
                     static_cast<float>(rb.x - lt.x),
                     static_cast<float>(rb.y - lt.y)};
  }
  auto pos =
      static_cast<wz::Property<wz::WzVec2D>*>(node->get_child(u"pos"))->get();
  auto width = static_cast<wz::Property<int32_t>*>(node->get_child(u"width"))
                   ->get();
  auto height = static_cast<wz::Property<int32_t>*>(node->get_child(u"height"))
                    ->get();
  return SDL_FRect{static_cast<float>(pos.x), static_cast<float>(pos.y),
                   static_cast<float>(width), static_cast<float>(height)};
}

// 画布按 origin 摆放：位置 = 锚点 - origin
SDL_FRect LoadOriginRect(SDL_Texture* texture, SDL_FPoint origin,
                         SDL_FPoint anchor) {
  if (texture == nullptr) {
    return SDL_FRect{anchor.x, anchor.y, 0, 0};
  }
  return SDL_FRect{
      anchor.x - origin.x,
      anchor.y - origin.y,
      static_cast<float>(texture->w),
      static_cast<float>(texture->h),
  };
}

void DrawText(const std::u16string& text, const SDL_FRect& rect, uint8_t size,
              SDL_Color color, bool centered) {
  if (text.empty()) {
    return;
  }
  Freetype::LoadSize(size);
  Freetype::LoadColor(color.r, color.g, color.b, color.a);
  if (centered) {
    Freetype::DrawCstr(text, rect.x, rect.y, rect.w, kLineSpacing);
  } else {
    Freetype::DrawStr(text, rect.x, rect.y, rect.w, kLineSpacing);
  }
  Freetype::LoadColor(255, 255, 255, 255);
}

}  // namespace

std::u16string OptionUiSystem::LoadTabName(uint8_t tab) {
  switch (static_cast<TabEnum>(tab)) {
    case TabEnum::kGraphic: {
      return u"graphic";
    }
    case TabEnum::kSound: {
      return u"sound";
    }
    case TabEnum::kGame: {
      return u"game";
    }
    case TabEnum::kSocial: {
      return u"social";
    }
    case TabEnum::kController: {
      return u"controller";
    }
  }
  return u"graphic";
}

wz::Node* OptionUiSystem::LoadTabNode(uint8_t tab) {
  if (WzResource::ui_ == nullptr) {
    return nullptr;
  }
  return WzResource::ui_->find(u"Options.img/" + LoadTabName(tab));
}

std::u16string OptionUiSystem::MakeKey(uint8_t tab, const std::u16string& name) {
  return LoadTabName(tab) + u"/" + name;
}

std::flat_map<std::u16string, int32_t>& OptionUiSystem::Values() {
  static std::flat_map<std::u16string, int32_t> values;
  return values;
}

// 解析一个 tab 下所有可交互 / 可显示的元素
std::vector<OptionUiSystem::Element> OptionUiSystem::LoadElements(uint8_t tab) {
  std::vector<Element> elements;
  auto* node = LoadTabNode(tab);
  if (node == nullptr || node->get_children() == nullptr) {
    return elements;
  }
  for (const auto& [key, list] : *node->get_children()) {
    if (list.empty() || list[0] == nullptr) {
      continue;
    }
    auto colon = key.find(u':');
    if (colon == std::u16string::npos) {
      continue;
    }
    Element element;
    element.kind = key.substr(0, colon);
    element.name = key.substr(colon + 1);
    element.node = list[0];
    if (element.kind == u"layer") {
      continue;  // 背景图单独画
    }
    if (element.kind == u"staticText") {
      // 标签用 SID 到 StringTable 取文案，数值类直接写 text
      if (auto* sid = element.node->get_child(u"SID")) {
        element.text = LoadText(
            static_cast<wz::Property<std::u16string>*>(sid)->get());
      } else if (auto* text = element.node->get_child(u"text")) {
        element.text =
            static_cast<wz::Property<std::u16string>*>(text)->get();
      }
      element.f_type = static_cast<wz::Property<int32_t>*>(
                           element.node->get_child(u"fType"))
                           ->get();
      element.rect = LoadRect(element.node);
    } else if (element.kind == u"check" || element.kind == u"edit") {
      element.rect = LoadRect(element.node);
    } else if (element.kind == u"slider") {
      auto lt =
          static_cast<wz::Property<wz::WzVec2D>*>(element.node->get_child(u"lt"))
              ->get();
      element.width = static_cast<wz::Property<int32_t>*>(
                          element.node->get_child(u"width"))
                          ->get();
      element.rect = SDL_FRect{static_cast<float>(lt.x),
                               static_cast<float>(lt.y),
                               static_cast<float>(element.width), kComboItemH};
    } else if (element.kind == u"combo") {
      element.rect = LoadRect(element.node);
      auto* contents = element.node->get_child(u"contents");
      if (contents != nullptr && contents->get_children() != nullptr) {
        for (const auto& [group_key, group] : *contents->get_children()) {
          (void)group_key;
          if (group.empty() || group[0] == nullptr) {
            continue;
          }
          for (const auto& [index, entry] : *group[0]->get_children()) {
            (void)index;
            if (entry.empty() || entry[0] == nullptr) {
              continue;
            }
            element.selects.push_back(
                static_cast<wz::Property<std::u16string>*>(
                    entry[0]->get_child(u"string"))
                    ->get());
          }
        }
      }
    } else {
      continue;  // button 等没有 lt/rb 的元素
    }
    elements.push_back(std::move(element));
  }
  return elements;
}

// SID -> String/StringTable.img 里的文案
std::u16string OptionUiSystem::LoadText(const std::u16string& sid) {
  if (sid.empty() || WzResource::string_ == nullptr) {
    return u"";
  }
  auto* node = WzResource::string_->find(u"StringTable.img/" + sid);
  if (node == nullptr || node->type != wz::Type::String) {
    return u"";
  }
  return static_cast<wz::Property<std::u16string>*>(node)->get();
}

int32_t OptionUiSystem::DefaultValue(const Element& element) {
  if (element.kind == u"slider") {
    // 透明度类默认全不透明，其余通道默认 50
    if (element.name == u"alphaMine" || element.name == u"alphaOther") {
      return 100;
    }
    return 50;
  }
  if (element.kind == u"edit") {
    if (element.name == u"inarow") {
      return 5;
    }
    if (element.name == u"memorySize") {
      return 2;
    }
  }
  return 0;
}

int32_t OptionUiSystem::Value(uint8_t tab, const std::u16string& name) {
  auto& values = Values();
  auto it = values.find(MakeKey(tab, name));
  if (it != values.end()) {
    return it->second;
  }
  return 0;
}

void OptionUiSystem::SetValue(uint8_t tab, const std::u16string& name,
                              int32_t val) {
  Values()[MakeKey(tab, name)] = val;
}

void OptionUiSystem::Load() {
  auto& values = Values();
  auto& saved = GameSaveSystemInstance::save_.options;
  for (uint8_t tab = 0; tab < kTabCount; ++tab) {
    for (const auto& element : LoadElements(tab)) {
      auto key = MakeKey(tab, element.name);
      auto it = saved.find(key);
      if (it != saved.end()) {
        values[key] = it->second;
      } else {
        values.try_emplace(key, DefaultValue(element));
      }
    }
  }
  loaded_ = true;
  Apply();
}

void OptionUiSystem::Save() {
  auto& saved = GameSaveSystemInstance::save_.options;
  for (const auto& [key, val] : Values()) {
    saved[key] = val;
  }
  Apply();
}

uint8_t OptionUiSystem::HpWarnPercent() {
  return static_cast<uint8_t>(std::clamp(
      Value(static_cast<uint8_t>(TabEnum::kGame), u"flashHP"), 0, kSliderMax));
}

uint8_t OptionUiSystem::MpWarnPercent() {
  return static_cast<uint8_t>(std::clamp(
      Value(static_cast<uint8_t>(TabEnum::kGame), u"flashMP"), 0, kSliderMax));
}

uint8_t OptionUiSystem::EnemyInfo() {
  return static_cast<uint8_t>(std::clamp(
      Value(static_cast<uint8_t>(TabEnum::kGame), u"monster"), 0, 2));
}

uint8_t OptionUiSystem::ChatFontSize() {
  // 滑块 0..100 映射到 10..16 号字
  auto val = std::clamp(
      Value(static_cast<uint8_t>(TabEnum::kSocial), u"chatFontsize"), 0,
      kSliderMax);
  return static_cast<uint8_t>(10 + (val * 6 + 50) / 100);
}

// 把选项推给真正使用它们的系统
void OptionUiSystem::Apply() {
  const auto sound = static_cast<uint8_t>(TabEnum::kSound);

  auto master = Value(sound, u"volMV");
  if (Value(sound, u"muteMV") != 0) {
    master = 0;
  }
  auto bgm = Value(sound, u"volBG");
  if (Value(sound, u"muteBG") != 0) {
    bgm = 0;
  }
  auto sfx = Value(sound, u"volSE");
  if (Value(sound, u"muteSE") != 0) {
    sfx = 0;
  }
  AudioGameInstance::SetVolume(static_cast<float>(master) / kSliderMax,
                               static_cast<float>(bgm) / kSliderMax,
                               static_cast<float>(sfx) / kSliderMax);

  ChatLogUiSystem::chat_font_size_ = ChatFontSize();
}

void OptionUiSystem::RenderBackgrnd() {
  static auto backgrnd =
      WzResource::LoadTexture(WzResource::ui_->find(u"Options.img/backgrnd"));
  SDL_FRect pos_rect{std::round(pos_.x), std::round(pos_.y), kPanelW, kPanelH};
  if (backgrnd != nullptr) {
    pos_rect.w = static_cast<float>(backgrnd->w);
    pos_rect.h = static_cast<float>(backgrnd->h);
    SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
  }

  // 当前 tab 的内容区背景（layer:backgrnd）
  auto* node = LoadTabNode(active_tab_);
  if (node == nullptr) {
    return;
  }
  auto* layer = node->get_child(u"layer:backgrnd");
  if (layer == nullptr) {
    return;
  }
  auto texture = WzResource::LoadTexture(layer);
  auto origin =
      static_cast<wz::Property<wz::WzVec2D>*>(layer->get_child(u"origin"))
          ->get();
  auto rect = LoadOriginRect(
      texture,
      SDL_FPoint{static_cast<float>(origin.x), static_cast<float>(origin.y)},
      SDL_FPoint{pos_rect.x, pos_rect.y});
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &rect);
}

// 静态文字：标签/说明用 SID 取文案，数值由元素名推出来。
// 左侧标签栏是粉色底（白字），右侧是米色底（深色字）。
void OptionUiSystem::RenderContent() {
  const SDL_Color label_color{255, 255, 255, 255};
  const SDL_Color small_color{70, 60, 60, 255};
  for (const auto& element : LoadElements(active_tab_)) {
    SDL_FRect rect = element.rect;
    rect.x += std::round(pos_.x);
    rect.y += std::round(pos_.y);
    if (element.kind == u"staticText") {
      bool is_label = element.f_type != 283;
      DrawText(element.text, rect, is_label ? kLabelSize : kSmallSize,
               is_label ? label_color : small_color, !is_label);
    } else if (element.kind == u"edit") {
      auto text = std::to_string(Value(active_tab_, element.name));
      DrawText(std::u16string{text.begin(), text.end()}, rect, kLabelSize,
               small_color, true);
    }
  }
}

void OptionUiSystem::RenderComboList() {
  if (!open_combo_.has_value()) {
    return;
  }
  for (const auto& element : LoadElements(active_tab_)) {
    if (element.kind != u"combo" || element.name != *open_combo_) {
      continue;
    }
    SDL_FRect rect = element.rect;
    rect.x = std::round(pos_.x + rect.x);
    rect.y = std::round(pos_.y + rect.y);
    SDL_FRect list{rect.x, rect.y + rect.h, rect.w,
                   static_cast<float>(element.selects.size()) * kComboItemH};
    SDL_SetRenderDrawColor(Window::renderer_, 245, 240, 230, 255);
    SDL_RenderFillRect(Window::renderer_, &list);
    SDL_SetRenderDrawColor(Window::renderer_, 60, 50, 40, 255);
    SDL_RenderRect(Window::renderer_, &list);

    auto val = Value(active_tab_, element.name);
    for (size_t i = 0; i < element.selects.size(); ++i) {
      SDL_FRect item{list.x + 1, list.y + static_cast<float>(i) * kComboItemH,
                     list.w - 2, kComboItemH};
      bool hover = SDL_PointInRectFloat(&Window::mouse_pos_, &item);
      if (hover || static_cast<int32_t>(i) == val) {
        SDL_SetRenderDrawColor(Window::renderer_, hover ? 120 : 190, 170, 140,
                               255);
        SDL_RenderFillRect(Window::renderer_, &item);
      }
      DrawText(element.selects[i], item, kSmallSize,
               SDL_Color{40, 30, 20, 255}, false);
    }
    break;
  }
}

bool OptionUiSystem::Render() {
  RenderBackgrnd();
  RenderContent();
  RenderUI();
  RenderComboList();
  return true;
}

UIComponents& OptionUiSystem::GetComponent() {
  components_.clear();
  const float px = std::round(pos_.x);
  const float py = std::round(pos_.y);
  const bool top = CursorGameInstance::cursor_ui_ == Render &&
                   !CursorGameInstance::modal_overlay_;

  // 三个窗口按钮：位置由 wz 里 normal/0 的 origin 给出
  const std::array<std::pair<const char16_t*, Button::Callback>, 3> buttons = {{
      {u"button:OK", EventButtonOk},
      {u"button:Cancel", EventButtonCancel},
      {u"button:Close", EventButtonClose},
  }};
  for (const auto& [name, callback] : buttons) {
    auto* node = WzResource::ui_->find(u"Options.img/" + std::u16string{name});
    if (node == nullptr) {
      continue;
    }
    auto* normal = node->get_child(u"normal");
    if (normal == nullptr || normal->get_child(u"0") == nullptr) {
      continue;
    }
    auto* canvas = normal->get_child(u"0");
    auto origin =
        static_cast<wz::Property<wz::WzVec2D>*>(canvas->get_child(u"origin"))
            ->get();
    Button button;
    button.node = node;
    button.rect = LoadOriginRect(
        WzResource::LoadTexture(canvas),
        SDL_FPoint{static_cast<float>(origin.x), static_cast<float>(origin.y)},
        SDL_FPoint{px, py});
    button.enable = true;
    button.focused = top;
    button.callback = callback;
    components_.push_back(button);
  }

  // tab 条：绘制与命中测试都交给 Tab 组件
  auto* tab_node = WzResource::ui_->find(u"Options.img/tab");
  if (tab_node != nullptr && tab_node->get_child(u"disabled") != nullptr) {
    std::vector<SDL_FRect> tab_rects;
    for (uint8_t i = 0; i < kTabCount; ++i) {
      auto* canvas =
          tab_node->get_child(u"disabled")->get_child(std::to_string(i));
      if (canvas == nullptr) {
        continue;
      }
      auto origin =
          static_cast<wz::Property<wz::WzVec2D>*>(canvas->get_child(u"origin"))
              ->get();
      tab_rects.push_back(LoadOriginRect(
          WzResource::LoadTexture(canvas),
          SDL_FPoint{static_cast<float>(origin.x), static_cast<float>(origin.y)},
          SDL_FPoint{px, py}));
    }
    if (!tab_rects.empty()) {
      components_.push_back(
          Tab{tab_node, u"enabled", u"disabled", tab_rects, active_tab_});
    }
  }

  // 控件
  for (const auto& element : LoadElements(active_tab_)) {
    SDL_FRect rect = element.rect;
    rect.x += px;
    rect.y += py;
    if (element.kind == u"check") {
      CheckBox box{.rect = rect,
                   .checked = Value(active_tab_, element.name) != 0};
      components_.push_back(box);
    } else if (element.kind == u"slider") {
      Slider slider(static_cast<uint32_t>(std::clamp(
                        Value(active_tab_, element.name), 0, kSliderMax)),
                    kSliderMax, element.width);
      slider.pos = {rect.x, rect.y};
      components_.push_back(slider);
    } else if (element.kind == u"combo") {
      ComboBox combo;
      combo.open = false;  // 展开列表由 RenderComboList 画
      combo.w = static_cast<uint32_t>(rect.w);
      combo.selects = element.selects;
      if (!combo.selects.empty()) {
        auto val = std::clamp(Value(active_tab_, element.name), 0,
                              static_cast<int32_t>(combo.selects.size()) - 1);
        combo.val = static_cast<uint8_t>(val);
        combo.selected = combo.selects[static_cast<size_t>(val)];
      }
      combo.rect = rect;
      components_.push_back(combo);
    }
  }

  return components_;
}

bool OptionUiSystem::EventUI(SDL_Event* event) {
  auto mouse = SDL_FPoint{event->button.x, event->button.y};
  auto& components = GetComponent();

  // tab 优先
  for (auto& component : components) {
    auto* tab = std::get_if<Tab>(&component);
    if (tab == nullptr) {
      continue;
    }
    if (auto i = tab->Click(mouse)) {
      if (*i != active_tab_) {
        active_tab_ = *i;
        open_combo_ = std::nullopt;
        AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      }
      return true;
    }
  }

  // 已经展开的下拉列表优先吃掉点击
  if (open_combo_.has_value()) {
    for (const auto& element : LoadElements(active_tab_)) {
      if (element.kind != u"combo" || element.name != *open_combo_) {
        continue;
      }
      if (auto i = ClickCombo(element, mouse)) {
        SetValue(active_tab_, element.name, *i);
        AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
        Apply();
      }
      open_combo_ = std::nullopt;
      return true;
    }
  }

  for (const auto& element : LoadElements(active_tab_)) {
    SDL_FRect rect = element.rect;
    rect.x += std::round(pos_.x);
    rect.y += std::round(pos_.y);
    if (!SDL_PointInRectFloat(&mouse, &rect)) {
      continue;
    }
    if (element.kind == u"check") {
      auto val = Value(active_tab_, element.name);
      SetValue(active_tab_, element.name, val == 0 ? 1 : 0);
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      Apply();
      return true;
    }
    if (element.kind == u"combo") {
      open_combo_ = element.name;
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }

  // 按钮最后处理，避免被控件抢掉
  if (DispatchButtonClick(components, false)) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
    return true;
  }
  return false;
}

std::optional<uint8_t> OptionUiSystem::ClickCombo(const Element& element,
                                                  SDL_FPoint mouse) {
  SDL_FRect rect = element.rect;
  rect.x += std::round(pos_.x);
  rect.y += std::round(pos_.y);
  SDL_FRect list{rect.x, rect.y + rect.h, rect.w,
                 static_cast<float>(element.selects.size()) * kComboItemH};
  if (!SDL_PointInRectFloat(&mouse, &list)) {
    return std::nullopt;
  }
  auto index = static_cast<size_t>((mouse.y - list.y) / kComboItemH);
  if (index >= element.selects.size()) {
    return std::nullopt;
  }
  return static_cast<uint8_t>(index);
}

// 拖动 / 点击滑块，边拖边改
void OptionUiSystem::EventSlider(SDL_Event* event) {
  auto mouse = SDL_FPoint{event->motion.x, event->motion.y};
  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    mouse = SDL_FPoint{event->button.x, event->button.y};
  }
  for (const auto& element : LoadElements(active_tab_)) {
    if (element.kind != u"slider") {
      continue;
    }
    SDL_FRect rect = element.rect;
    rect.x += std::round(pos_.x);
    rect.y += std::round(pos_.y);
    if (!SDL_PointInRectFloat(&mouse, &rect)) {
      continue;
    }
    auto percent = std::clamp((mouse.x - rect.x) / rect.w, 0.0f, 1.0f);
    SetValue(active_tab_, element.name,
             static_cast<int32_t>(std::round(percent * kSliderMax)));
    dragging_slider_ = true;
    Apply();
    return;
  }
}

SDL_FPoint OptionUiSystem::LoadWh() {
  return {kPanelW, kPanelH};
}

void OptionUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it == System::render_systems_.end()) {
    return;
  }
  if (!loaded_) {
    Load();
  }
  auto wh = LoadWh();
  auto& camera = CameraGameInstance::camera_;
  pos_.x = std::round((camera.w - wh.x) / 2);
  pos_.y = std::round((camera.h - wh.y) / 2);
  open_combo_ = std::nullopt;
  dragging_slider_ = false;

  System::render_systems_.insert(it, Render);
  System::event_systems_.insert(System::event_systems_.begin(), Event);
}

void OptionUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  open_combo_ = std::nullopt;
  dragging_slider_ = false;
}

void OptionUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void OptionUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect{pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos{event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
}

void OptionUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
}

void OptionUiSystem::EventDragMove(SDL_Event* event) {
  if (!drag_.has_value()) {
    return;
  }
  auto& camera = CameraGameInstance::camera_;
  auto wh = LoadWh();
  pos_.x = std::clamp(event->motion.x + drag_->x, 0.0f, camera.w - wh.x);
  pos_.y = std::clamp(event->motion.y + drag_->y, 0.0f, camera.h - wh.y);
}

void OptionUiSystem::EventButtonOk() {
  Save();
  Close();
}

void OptionUiSystem::EventButtonCancel() {
  // 丢弃本次改动：重新按存档载入
  loaded_ = false;
  Load();
  Close();
}

void OptionUiSystem::EventButtonClose() {
  Save();
  Close();
}

void OptionUiSystem::EventMotionBtAudio(SDL_Event* event) {
  (void)event;
  static int8_t hover = -1;
  int8_t now = static_cast<int8_t>(ButtonHoverIndex(GetComponent()));
  if (now != -1 && hover != now) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  hover = now;
}

bool OptionUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT &&
          CursorGameInstance::cursor_ui_ == Render) {
        EventTop();
        EventDragStart(event);
        EventSlider(event);
        r = false;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        dragging_slider_ = false;
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventUI(event);
        }
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotionBtAudio(event);
      EventDragMove(event);
      if (dragging_slider_ && (Window::mouse_state_ & SDL_BUTTON_LMASK)) {
        EventSlider(event);
      }
      break;
    }
    default: {
      break;
    }
  }
  return r;
}

void OptionUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool OptionUiSystem::CursorIn() {
  auto wh = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, wh.x, wh.y};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

}  // namespace sdlms
