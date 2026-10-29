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
#include <iterator>
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
constexpr uint8_t kLabelSize = 11;
constexpr uint8_t kSmallSize = 11;
// 行距（按字体行高倍数）
constexpr float kLineSpacing = 1.0f;
// 下拉列表每项高度，也用作滑块轨道的点击高度
constexpr float kComboItemH = 16.0f;
// 下拉列表最下面的留白
constexpr float kComboListBottomPad = 4.0f;
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

int32_t LoadInt(wz::Node* node, const char16_t* name, int32_t fallback) {
  auto* child = node->get_child(name);
  if (child == nullptr) {
    return fallback;
  }
  return static_cast<wz::Property<int32_t>*>(child)->get();
}

// 单段文字的按词换行；放不下时优先在最后一个空格处断开。
std::vector<std::u16string> WrapParagraph(const std::u16string& paragraph,
                                          float width, bool wrap) {
  std::vector<std::u16string> lines;
  if (!wrap) {
    lines.emplace_back(paragraph);
    return lines;
  }
  std::u16string line;
  size_t last_space = std::u16string::npos;
  for (char16_t c : paragraph) {
    if (c == u' ') {
      last_space = line.size();
    }
    line.push_back(c);
    if (width <= 0.0f || line.size() <= 1 || Freetype::LoadW(line) <= width) {
      continue;
    }
    if (last_space != std::u16string::npos && last_space > 0) {
      std::u16string rest = line.substr(last_space + 1);
      line.resize(last_space);
      lines.emplace_back(std::move(line));
      line = std::move(rest);
    } else {
      std::u16string rest(1, c);
      line.pop_back();
      lines.emplace_back(std::move(line));
      line = std::move(rest);
    }
    last_space = line.find_last_of(u' ');
  }
  if (!line.empty() || lines.empty()) {
    lines.emplace_back(std::move(line));
  }
  return lines;
}

// 按 WZ 里的 alignHorizontal / alignVertical 排版：
// 0 = 左/上，1 = 右/下，2 = 中。多行文本逐行对齐。
std::vector<std::u16string> WrapText(const std::u16string& text, float width,
                                     bool wrap) {
  std::vector<std::u16string> lines;
  std::u16string paragraph;
  for (char16_t c : text) {
    if (c == u'\r') {
      continue;
    }
    if (c == u'\n') {
      auto part = WrapParagraph(paragraph, width, wrap);
      lines.insert(lines.end(), std::make_move_iterator(part.begin()),
                   std::make_move_iterator(part.end()));
      paragraph.clear();
      continue;
    }
    paragraph.push_back(c);
  }
  auto part = WrapParagraph(paragraph, width, wrap);
  lines.insert(lines.end(), std::make_move_iterator(part.begin()),
               std::make_move_iterator(part.end()));
  return lines;
}

// 单行超宽时水平压缩绘制，保持字号不变且不超出 WZ 文本框。
void DrawCompressedLine(const std::u16string& line, float x, float y,
                        float max_w) {
  const float total = Freetype::LoadW(line);
  if (total <= max_w || total <= 0.0f) {
    Freetype::DrawLine(line, x, y);
    return;
  }
  const float scale = max_w / total;
  float cursor = x;
  for (char16_t c : line) {
    const std::u16string glyph(1, c);
    Freetype::DrawLine(glyph, cursor, y);
    cursor += Freetype::LoadW(glyph) * scale;
  }
}

void DrawAlignedText(const std::u16string& text, const SDL_FRect& rect,
                     uint8_t size, SDL_Color color, int32_t align_h,
                     int32_t align_v, bool wrap) {
  if (text.empty()) {
    return;
  }
  Freetype::LoadSize(size);
  Freetype::LoadColor(color.r, color.g, color.b, color.a);

  const auto lines = WrapText(text, rect.w, wrap);
  const float line_h = Freetype::LoadLh() * kLineSpacing;
  const float block_h = line_h * static_cast<float>(lines.size());
  float y = rect.y;
  if (align_v == 1) {
    y += (rect.h - block_h) / 2.0f;
  } else if (align_v == 2) {
    y += rect.h - block_h;
  }

  for (const auto& line : lines) {
    const float w = Freetype::LoadW(line);
    const float draw_w = std::min(w, rect.w);
    float x = rect.x;
    if (align_h == 1) {
      x += rect.w - draw_w;
    } else if (align_h == 2) {
      x += (rect.w - draw_w) / 2.0f;
    }
    DrawCompressedLine(line, x, y, rect.w);
    y += line_h;
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
      // 参考客户端里这些标签是固定两行；本 WZ 的 StringTable 用空格代替了
      // 换行，这里补回来，保证行数和参考图一致。
      static const std::flat_map<std::u16string, std::u16string> kLabelLines = {
          {u"BackgroundQualityLabel", u"Background\nQuality"},
          {u"DamageSkinEffects", u"Damage Skin\nEffects"},
          {u"DamageSkinDisplayLabel", u"Damage Skin\nDisplay"},
          {u"ScreenshotFileFormatLabel", u"Screenshot File\nFormat"},
          {u"MemoryMappedIOLabel", u"Memory-mapped\nI/O"},
      };
      if (auto it = kLabelLines.find(element.name); it != kLabelLines.end()) {
        element.text = it->second;
      }
      element.f_type = static_cast<wz::Property<int32_t>*>(
                           element.node->get_child(u"fType"))
                           ->get();
      element.align_h = LoadInt(element.node, u"alignHorizontal", 0);
      element.align_v = LoadInt(element.node, u"alignVertical", 1);
      element.rect = LoadRect(element.node);
    } else if (element.kind == u"check") {
      element.rect = LoadRect(element.node);
    } else if (element.kind == u"edit") {
      // WZ 里连续截图 checkbox 和 burst 数字都叫 inarow，存档键会冲突；
      // burst 数字单独用 burstCount。
      if (element.name == u"inarow") {
        element.name = u"burstCount";
      }
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
    } else if (element.kind == u"button") {
      // Game tab 的 Burst / Memory 加减按钮：按钮位置由 normal/0 的 origin 给出
      auto* normal = element.node->get_child(u"normal");
      auto* canvas =
          normal == nullptr ? nullptr : normal->get_child(u"0");
      if (canvas == nullptr) {
        continue;
      }
      auto* origin = canvas->get_child(u"origin");
      if (origin == nullptr) {
        continue;
      }
      auto o =
          static_cast<wz::Property<wz::WzVec2D>*>(origin)->get();
      auto* texture = WzResource::LoadTexture(canvas);
      element.rect = SDL_FRect{-static_cast<float>(o.x),
                               -static_cast<float>(o.y),
                               static_cast<float>(texture->w),
                               static_cast<float>(texture->h)};
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
    if (element.name == u"burstCount") {
      return 5;
    }
    if (element.name == u"memorySize") {
      return 16;
    }
  }
  if (element.kind == u"combo") {
    // 参考图默认 Show Name and HP（第 3 项）
    if (element.name == u"monster") {
      return 3;
    }
  }
  if (element.kind == u"check") {
    // 自动截图默认开启
    if (element.name == u"auto") {
      return 1;
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
      Value(static_cast<uint8_t>(TabEnum::kGame), u"monster"), 0, 3));
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
      // 标签左右各留 3px，避免字号加大后被压缩到紧贴/切掉粉色条边缘
      if (is_label) {
        rect.x += 3.0f;
        rect.w = std::max(0.0f, rect.w - 6.0f);
      }
      // 说明文字和复选框同一行：把文字基线略上移，和参考图中
      // checkbox 与文字的中心对齐一致。
      if (!is_label && element.align_h == 0) {
        rect.y -= 2.0f;
      }
      DrawAlignedText(element.text, rect, is_label ? kLabelSize : kSmallSize,
                      is_label ? label_color : small_color, element.align_h,
                      element.align_v, !is_label && element.align_h != 1);
    } else if (element.kind == u"edit") {
      auto text = std::to_string(Value(active_tab_, element.name));
      // 编辑框里的数字靠左，和外层白色输入框的左边距一致
      SDL_FRect text_rect = rect;
      text_rect.x += 5.0f;
      text_rect.w = std::max(0.0f, text_rect.w - 5.0f);
      DrawAlignedText(std::u16string{text.begin(), text.end()}, text_rect,
                      kLabelSize, small_color, 0, 1, false);
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

    // 下拉框和展开列表都交给 ComboBox 组件绘制（在 RenderUI 之后调用，
    // 因此列表始终盖在其它控件上面）。
    ComboBox combo;
    combo.open = true;
    combo.w = static_cast<uint32_t>(rect.w);
    combo.selects = element.selects;
    combo.rect = rect;
    if (!combo.selects.empty()) {
      auto val = std::clamp(Value(active_tab_, element.name), 0,
                            static_cast<int32_t>(combo.selects.size()) - 1);
      combo.val = static_cast<uint8_t>(val);
      combo.selected = combo.selects[static_cast<size_t>(val)];
    }
    combo.Render();
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
      slider.pos = {static_cast<int>(std::lround(rect.x)),
                    static_cast<int>(std::lround(rect.y))};
      components_.push_back(slider);
    } else if (element.kind == u"combo") {
      // 展开中的 combo 由 RenderComboList 用 ComboBox::Render 画，
      // 并且要盖在其它控件上面；这里先跳过，避免同一个框画两遍。
      if (open_combo_.has_value() && *open_combo_ == element.name) {
        continue;
      }
      ComboBox combo;
      combo.open = false;
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
    } else if (element.kind == u"button") {
      // Game tab 的 Burst / Memory 加减按钮
      const bool up = element.name.ends_with(u"Up");
      const bool memory = element.name.starts_with(u"memory");
      const std::u16string field =
          memory ? u"memorySize" : u"burstCount";
      const int32_t low = 2;
      const int32_t high = memory ? 64 : 10;
      const int32_t step = memory ? 2 : 1;
      const int32_t value = Value(active_tab_, field);
      Button button;
      button.node = element.node;
      button.rect = rect;
      button.enable = up ? value < high : value > low;
      button.focused = top;
      button.callback = [field, up, low, high, step]() {
        int32_t v = Value(active_tab_, field);
        v = up ? std::min(high, v + step) : std::max(low, v - step);
        SetValue(active_tab_, field, v);
        AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
        Apply();
      };
      components_.push_back(button);
    }
  }

  return components_;
}

bool OptionUiSystem::EventUI(SDL_Event* event) {
  auto mouse = SDL_FPoint{event->button.x, event->button.y};
  // 合成点击/鼠标移动丢失时，按钮命中应以本次点击坐标为准
  Window::mouse_pos_ = mouse;
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
    if (element.kind == u"button") {
      // Game tab 的 Burst / Memory 加减按钮，直接按点击坐标处理
      const bool up = element.name.ends_with(u"Up");
      const bool memory = element.name.starts_with(u"memory");
      const std::u16string field =
          memory ? u"memorySize" : u"burstCount";
      const int32_t low = 2;
      const int32_t high = memory ? 64 : 10;
      const int32_t step = memory ? 2 : 1;
      int32_t value = Value(active_tab_, field);
      if ((up && value < high) || (!up && value > low)) {
        value = up ? std::min(high, value + step)
                   : std::max(low, value - step);
        SetValue(active_tab_, field, value);
        AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
        Apply();
      }
      return true;
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
                 static_cast<float>(element.selects.size()) * kComboItemH +
                     kComboListBottomPad};
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
    // 与 Slider 的 thumb 纹理行程一致：左侧内缩 2px，右侧留 5px
    const float track_x = rect.x + 2.0f;
    const float track_w = std::max(1.0f, rect.w - 7.0f);
    auto percent =
        std::clamp((mouse.x - track_x) / track_w, 0.0f, 1.0f);
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
