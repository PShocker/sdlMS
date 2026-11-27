#include "src/client/system/ui/screen_notice_ui_system.h"

#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <cmath>
#include <cstdint>
#include <string>

namespace sdlms {

namespace {

// 095 CFloatNotice::CreateFloatNotice 以 nY=160（屏幕顶部原点）放置公告条，
// 三段底图统一按 nAlpha=192 绘制。
constexpr float kNoticeY = 160;
constexpr uint8_t kBackgrndAlpha = 192;

std::u16string IntToU16(int value) {
  auto s = std::to_string(value);
  return {s.begin(), s.end()};
}

}  // namespace

void ScreenNoticeUiSystem::RenderNotice(const Notice& notice) {
  auto node = WzResource::ui_->find(u"UIWindow.img/FloatNotice/" +
                                    IntToU16(notice.type));
  if (node == nullptr) {
    return;
  }
  auto* left_node = node->get_child(u"0");
  auto* middle_node = node->get_child(u"1");
  auto* right_node = node->get_child(u"2");
  if (left_node == nullptr || middle_node == nullptr || right_node == nullptr) {
    return;
  }
  auto left = WzResource::LoadTexture(left_node);
  auto middle = WzResource::LoadTexture(middle_node);
  auto right = WzResource::LoadTexture(right_node);

  // 095 String[998]="y"：文字相对公告条顶部的纵向偏移。
  int text_y = 0;
  if (auto* y = node->get_child(u"y"); y != nullptr) {
    text_y = static_cast<wz::Property<int32_t>*>(y)->get();
  }

  // 095 未提供 fontColor 的类型沿用字体默认色（深色底图配白字）。
  SDL_Color color{255, 255, 255, 255};
  if (auto* fc = node->get_child(u"fontColor"); fc != nullptr) {
    auto v = static_cast<uint32_t>(
        static_cast<wz::Property<int32_t>*>(fc)->get());
    color = SDL_Color{
        static_cast<uint8_t>((v >> 16) & 0xFF),
        static_cast<uint8_t>((v >> 8) & 0xFF),
        static_cast<uint8_t>(v & 0xFF),
        static_cast<uint8_t>((v >> 24) & 0xFF),
    };
  }

  Freetype::LoadSize(12);
  Freetype::LoadBold(false);
  Freetype::LoadAligned(true);
  auto text_w = Freetype::LoadW(notice.text);

  // 095：nPlateNo = ceil(nTextWidth / nPlateWidth)，总宽 = 左端 + 中段*片数 + 右端。
  auto plate_w = static_cast<float>(middle->w);
  int plate_no = 0;
  if (text_w > 0 && plate_w > 0) {
    plate_no = static_cast<int>(std::ceil(text_w / plate_w));
  }
  auto plate_total = plate_no * plate_w;
  auto total_w = left->w + plate_total + right->w;

  auto x = std::floor((CameraGameInstance::camera_.w - total_w) / 2);
  auto y = kNoticeY;

  SDL_SetTextureAlphaMod(left, kBackgrndAlpha);
  SDL_SetTextureAlphaMod(middle, kBackgrndAlpha);
  SDL_SetTextureAlphaMod(right, kBackgrndAlpha);

  SDL_FRect rect{
      x,
      y,
      static_cast<float>(left->w),
      static_cast<float>(left->h),
  };
  SDL_RenderTexture(Window::renderer_, left, nullptr, &rect);

  rect = {
      x + left->w,
      y,
      plate_total,
      static_cast<float>(middle->h),
  };
  SDL_RenderTextureTiled(Window::renderer_, middle, nullptr, 1, &rect);

  rect = {
      x + left->w + plate_total,
      y,
      static_cast<float>(right->w),
      static_cast<float>(right->h),
  };
  SDL_RenderTexture(Window::renderer_, right, nullptr, &rect);

  SDL_SetTextureAlphaMod(left, 255);
  SDL_SetTextureAlphaMod(middle, 255);
  SDL_SetTextureAlphaMod(right, 255);

  Freetype::LoadColor(color.r, color.g, color.b, color.a);
  auto text_x = x + left->w + (plate_total - text_w) / 2;
  Freetype::DrawLine(notice.text, text_x, y + text_y);
}

void ScreenNoticeUiSystem::FloatNotice(const std::u16string& text, int type,
                                       uint64_t duration) {
  if (text.empty()) {
    DeleteFloatNotice();
    return;
  }
  // 095 CUIStatusBar::FloatNotice 会先 SetItemMsg(0) 清掉道具消息。
  DeleteItemMsg();
  Notice notice;
  notice.text = text;
  notice.type = type;
  notice.expire = duration == 0 ? 0 : Window::dt_now_ + duration;
  float_notice_ = std::move(notice);
}

void ScreenNoticeUiSystem::DeleteFloatNotice() { float_notice_.reset(); }

bool ScreenNoticeUiSystem::SetItemMsg(const std::u16string& text, int type,
                                      uint64_t duration) {
  if (text.empty()) {
    DeleteItemMsg();
    return true;
  }
  // 095：quiz/float/item 任一存在时不再创建新的道具消息。
  if (float_notice_.has_value() || item_msg_.has_value()) {
    return false;
  }
  Notice notice;
  notice.text = text;
  notice.type = type;
  notice.expire = duration == 0 ? 0 : Window::dt_now_ + duration;
  item_msg_ = std::move(notice);
  return true;
}

void ScreenNoticeUiSystem::DeleteItemMsg() { item_msg_.reset(); }

void ScreenNoticeUiSystem::Clear() {
  float_notice_.reset();
  item_msg_.reset();
}

bool ScreenNoticeUiSystem::Render() {
  // 095 CUIStatusBar::Update：到期即删除，expire 为 0 表示常驻。
  if (float_notice_.has_value() && float_notice_->expire != 0 &&
      Window::dt_now_ > float_notice_->expire) {
    float_notice_.reset();
  }
  if (item_msg_.has_value() && item_msg_->expire != 0 &&
      Window::dt_now_ > item_msg_->expire) {
    item_msg_.reset();
  }

  if (float_notice_.has_value()) {
    RenderNotice(*float_notice_);
  }
  if (item_msg_.has_value()) {
    RenderNotice(*item_msg_);
  }
  return true;
}

}  // namespace sdlms
